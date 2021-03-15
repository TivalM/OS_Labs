#include <stdio.h>
#include <string.h>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <climits>
#include <iomanip>
#include <queue>
#include "BaseObjects.h"
#ifdef _WIN32
#include "getopt.h"
#else
#include <unistd.h>
#endif
using namespace std;


int readOneRandomInt(int brust);
void readAllProcess();
Event* getEvent();
void printLog(Event* evt, Event* newEvt);
void printEventQueue();
void simulation();
bool compareTwoEvent(Event* eventA, Event* eventB);


bool vFlag = true;
bool tFlag = false;
bool eFlag = true;
const char* sValue;
int quantumNum = 100000;
int maxProiNum = 4;
int randCount = 0;
char* inputFile = NULL;
char* randFile = NULL;
std::ifstream inInputFile;
std::ifstream inRandFile;

queue<Process*> processList;
deque<Event*> eventList;
Scheduler* scheduler;

bool CALL_SCHEDULER = false;
Process* CURRENT_RUNNING_PROCESS = nullptr;

int main(int argc, char** argv)
{
	inputFile = argv[1];
	randFile = argv[2];
	inInputFile.open(inputFile);
	inRandFile.open(randFile);
	inRandFile >> randCount;
	readAllProcess();
	sValue = "F";

	if (strcmp(sValue, "F") == 0) {
		scheduler = new FIFO();
	}
	simulation();
}

int readOneRandomInt(int seed) {
	int randInt = -1;
	if (!inRandFile.is_open() || inRandFile.eof()) {
		inRandFile.close();
		inRandFile.open(randFile);
		inRandFile >> randCount;
	}
	inRandFile >> randInt;
	if (randInt == -1) {
		//empty line
		return 1 + (readOneRandomInt(seed) % seed);
	}
	return 1 + (randInt % seed);
}

void readAllProcess() {
	int a, b, c, d;
	while (inRandFile.is_open() && !inRandFile.eof()) {
		a = b = c = d = -1;
		inInputFile >> a >> b >> c >> d;
		if (a != -1) {
			Process* process = new Process(a, b, c, d, readOneRandomInt(maxProiNum));
			processList.push(process);
			Event* evt = new Event(a, process, ProcessState::CREATED, ProcessState::READY);
			eventList.push_back(evt);
		}
		else {
			break;
		}
	}
}

void simulation() {
	Event* evt;
	int currentTime = 0;
	int timeDuration;
	while ((evt = getEvent()) != nullptr) {
		Process* proc = evt->process;
		currentTime = evt->timeStamp;
		timeDuration = currentTime - proc->timeLastStateStart;

		switch (evt->getTransitionType()) {
		case TransitionType::CREATE_TO_READY: {
			//add process into ready queue
			scheduler->addProcess(proc);
			CALL_SCHEDULER = true;
		}
											break;
		case TransitionType::BLOCKED_TO_READY: {
			//process consumed an io brust
			proc->IOTime += proc->currentIOBrust;
			proc->currentIOBrust = 0;
			proc->dynamicPrio = proc->staticPrio - 1;

			//add process into ready queue
			scheduler->addProcess(proc);
			proc->timeLastStateStart = currentTime;
			CALL_SCHEDULER = true;
		}
											 break;
		case TransitionType::READY_TO_RUNNING: {
			proc->cpuWaitingTime += timeDuration;
			//generate a cpu brust
			int cb = readOneRandomInt(proc->maxCpuBurst);
			cb = cb > proc->maxCpuBurst ? proc->maxCpuBurst : cb;
			proc->currentCpuBrust = cb;
			if (cb < quantumNum) {
				//insert event running to block
				Event* newEvt = new Event(currentTime + cb, proc, ProcessState::RUNNING, ProcessState::BLOCKED);
				eventList.push_back(newEvt);
				stable_sort(eventList.begin(), eventList.end(), compareTwoEvent);
				printLog(evt,newEvt);
			}
			else {
				//insert event running to preempt

			}
		}
											 break;
		case TransitionType::RUNNING_TO_BLOCK: {
			//process consumed a cpu brust
			if (proc->remainingCpuTime > proc->currentCpuBrust) {
				proc->remainingCpuTime -= proc->currentCpuBrust;
			}
			else {
				//process should terminate here 
				proc->finishAtTime = currentTime - (proc->currentCpuBrust - proc->remainingCpuTime);
				proc->remainingCpuTime = 0;
			}
			proc->currentCpuBrust = 0;

			if (proc->remainingCpuTime > 0) {
				//generate an io brust
				int ib = readOneRandomInt(proc->maxIOBurst);
				ib = ib > proc->maxIOBurst ? proc->maxIOBurst : ib;;
				proc->currentIOBrust = ib;
				//insert event block to ready
				Event* newEvt = new Event(currentTime + ib, proc, ProcessState::BLOCKED, ProcessState::READY);
				eventList.push_back(newEvt);
				stable_sort(eventList.begin(), eventList.end(), compareTwoEvent);
				printLog(evt,newEvt);
				CALL_SCHEDULER = true;
			}
		}
											 break;
		case TransitionType::TRANS_TO_PREEMPT: {
		}
											 break;
		default: {
		}
			   break;
		}

		if (CALL_SCHEDULER) {
			if (eventList[0]->timeStamp == currentTime)
				continue; //process next event from Event queue
		}
		CALL_SCHEDULER = false;
		if (CURRENT_RUNNING_PROCESS == nullptr) {
			CURRENT_RUNNING_PROCESS = scheduler->getNextProcess();
			if (CURRENT_RUNNING_PROCESS == nullptr) {
				continue;
			}
			else{
				// create event to make process runnable for same time.
				Event* newEvt = new Event(currentTime, CURRENT_RUNNING_PROCESS, ProcessState::READY, ProcessState::RUNNING);
				eventList.push_back(newEvt);
				stable_sort(eventList.begin(), eventList.end(), compareTwoEvent);
				printLog(evt, newEvt);
			}
		}

	}
}

Event* getEvent() {
	if (!eventList.empty()) {
		Event* event = eventList.front();
		eventList.pop_front();
		return event;
	}
	else {
		return nullptr;
	}
}

void printLog(Event* evt, Event* newEvt) {
	if (vFlag) {
		if (evt != nullptr){
			evt->printInfo();

		}
		if (eFlag) {
			if (newEvt != nullptr){
				cout << "AddEvent(" << newEvt->timeStamp << ":" << newEvt->process->pid << ":" << stateToString(newEvt->newState) << ")";
				cout << "====>";
				printEventQueue();
			}
		}
	}
}

void printEventQueue() {
	int eventQueueSize = eventList.size();
	for (int i = 0; i < eventQueueSize; i++) {
		cout << " (" << eventList[i]->timeStamp << ":" << eventList[i]->process->pid << ":" << stateToString(eventList[i]->newState) << ") ";
	}
	cout << endl;
}

bool compareTwoEvent(Event* eventA, Event* eventB) {
	return (eventA->timeStamp < eventB->timeStamp);
	if (eventA->timeStamp < eventB->timeStamp) {
		return true;
	}
	else if (eventA->timeStamp > eventB->timeStamp) {
		return false;
	}
	else  if (eventA->timeStamp == eventB->timeStamp) {
		//to modified
		return false;
	}
}