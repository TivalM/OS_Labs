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

bool vFlag = true;
bool tFlag = false;
bool eFlag = false;
char* sValue = NULL;
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
	if (strcmp(sValue, "F")) {
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
			Event* evt = new Event(a, process, ProcessState::CREATED, ProcessState::CREATED);
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
		case TransitionType::CREATE_TO_READY:
			Event* newEvt = new Event(currentTime, proc, ProcessState::CREATED, ProcessState::READY);
			eventList.push_back(newEvt);
			if (vFlag) {
				newEvt->printInfo();
			}
			stable_sort(eventList.begin(), eventList.end(), compareTwoEvent);
			break;
		case TransitionType::BLOCKED_TO_READY:
			CALL_SCHEDULER = true;
			break;
		case TransitionType::READY_TO_RUNNING:
			int cb = readOneRandomInt(proc->maxCpuBurst);
			cb = cb > proc->maxCpuBurst ? proc->maxCpuBurst : cb;;
			proc->currentCpuBrust = cb;
			if (cb < quantumNum) {
				//insert event running to block
				Event* newEvt = new Event(currentTime + cb, proc, ProcessState::RUNNING, ProcessState::BLOCKED);
				eventList.push_front(newEvt);
				if (vFlag) {
					newEvt->printInfo();
				}
			}
			else {
				//insert event running to pre

			}
			stable_sort(eventList.begin(), eventList.end(), compareTwoEvent);
			break;
		case TransitionType::RUNNING_TO_BLOCK:
			int ib = readOneRandomInt(proc->maxIOBurst);
			ib = ib > proc->maxIOBurst ? proc->maxIOBurst : ib;;
			proc->maxIOBurst = ib;
			//insert event block to ready
			Event* newEvt = new Event(currentTime + ib, proc, ProcessState::BLOCKED, ProcessState::READY);
			eventList.push_back(newEvt);
			if (vFlag) {
				newEvt->printInfo();
			}

			stable_sort(eventList.begin(), eventList.end(), compareTwoEvent);
			break;
		case TransitionType::TRANS_TO_PREEMPT:
			break;
		default:
			break;
		}

		if (CALL_SCHEDULER) {
			if (eventList[0]->timeStamp == currentTime)
				continue; //process next event from Event queue
		}
		CALL_SCHEDULER = false;
		if (CURRENT_RUNNING_PROCESS == nullptr) {
			CURRENT_RUNNING_PROCESS = scheduler->getNextProcess();
		}
		if (CURRENT_RUNNING_PROCESS == nullptr) {
			continue;
		}
		// create event to make process runnable for same time.
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

void printEventQueue() {
	int eventQueueSize = eventList.size();
	for (int i = 0; i < eventQueueSize; i++) {
		cout << "event " << i << " : ";
		eventList[i]->printInfo();
	}
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