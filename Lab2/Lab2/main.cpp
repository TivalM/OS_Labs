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
void printResult();

bool vFlag = true;
bool tFlag = false;
bool eFlag = false;
const char* sValue;
int quantumNum = 100000;
int maxProiNum = 4;
int randCount = 0;
const char* inputFile = NULL;
const char* randFile = NULL;
std::ifstream inInputFile;
std::ifstream inRandFile;

deque<Process*> processList;
deque<Event*> eventList;
Scheduler* scheduler;

bool CALL_SCHEDULER = false;
Process* CURRENT_RUNNING_PROCESS = nullptr;

int TotalSimTime = 0;
double CpuUtil = 0;
double IOUtil = 0;
double AvgTurnAround = 0;
double AvgWaitTime = 0;
double Throughoput = 0;
int runningIOCount = 0;
int ioStartTime;
int main(int argc, char** argv)
{
	//inputFile = argv[1];
	//randFile = argv[2];
	inputFile = "G:\\NYU\\OS\\Labs\\Lab2\\assignment\\lab2_sample\\input6";
	randFile = "G:\\NYU\\OS\\Labs\\Lab2\\assignment\\lab2_sample\\rfile";
	inInputFile.open(inputFile);
	inRandFile.open(randFile);
	inRandFile >> randCount;
	readAllProcess();
	sValue = "R";

	if (strcmp(sValue, "F") == 0) {
		scheduler = new FCFS();
	}
	else if (strcmp(sValue, "L") == 0) {
		scheduler = new LCFS();
	}
	else if (strcmp(sValue, "S") == 0) {
		scheduler = new SRTF();
	}
	else if (strcmp(sValue, "R") == 0) {
		scheduler = new FCFS();
		quantumNum = 5;
	}
	simulation();
	printResult();
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
			processList.push_back(process);
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
			printLog(evt, nullptr);
		}
											break;
		case TransitionType::BLOCKED_TO_READY: {
			//process consumed an io brust
			proc->IOTime += proc->currentIOBrust;
			proc->currentIOBrust = 0;
			proc->dynamicPrio = proc->staticPrio - 1;
			//add process into ready queue
			scheduler->addProcess(proc);
			CALL_SCHEDULER = true;
			if (runningIOCount > 0) {
				runningIOCount--;
				if (runningIOCount == 0) {
					//io end
					IOUtil += (double)currentTime - (double)ioStartTime;
				}
			}
			printLog(evt, nullptr);
		}
											 break;
		case TransitionType::READY_TO_RUNNING: {
			proc->cpuWaitingTime += timeDuration;
			int cb;
			if (proc->currentCpuBrust==0){
				//generate a cpu brust
				cb = readOneRandomInt(proc->maxCpuBurst);
				cb = cb > proc->maxCpuBurst ? proc->maxCpuBurst : cb;
				cb = cb > proc->remainingCpuTime ? proc->remainingCpuTime : cb;
				proc->currentCpuBrust = cb;
			}
			else{
				//process reaming cpu brust
				cb = proc->currentCpuBrust;
			}
			if (cb <= quantumNum) {
				//insert event running to block
				Event* newEvt = new Event(currentTime + cb, proc, ProcessState::RUNNING, ProcessState::BLOCKED);
				eventList.push_back(newEvt);
				stable_sort(eventList.begin(), eventList.end(), compareTwoEvent);
				printLog(evt, newEvt);
			}
			else {
				//insert event for preempt
				Event* newEvt = new Event(currentTime + quantumNum, proc, ProcessState::RUNNING, ProcessState::READY);
				eventList.push_back(newEvt);
				stable_sort(eventList.begin(), eventList.end(), compareTwoEvent);
				printLog(evt, newEvt);
			}
		}
											 break;
		case TransitionType::RUNNING_TO_BLOCK: {
			CURRENT_RUNNING_PROCESS = nullptr;
			//process consumed a cpu brust
			proc->remainingCpuTime -= proc->currentCpuBrust;
			if (proc->remainingCpuTime == 0) {
				//process should terminate here 
				proc->finishAtTime = currentTime;
				printLog(evt, nullptr);
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
				if (runningIOCount == 0) {
					ioStartTime = currentTime;
				}
				runningIOCount++;
				printLog(evt, newEvt);
				CALL_SCHEDULER = true;
			}
		}
											 break;
		case TransitionType::RUNNING_TO_READY: {
			//preempted by the scheduler
			CURRENT_RUNNING_PROCESS = nullptr;
			proc->remainingCpuTime -= timeDuration;
			proc->currentCpuBrust -= timeDuration;
			scheduler->addProcess(proc);
			printLog(evt, nullptr);
			CALL_SCHEDULER = true;
		}
											 break;
		default: {
		}
			   break;
		}
		proc->timeLastStateStart = currentTime;
		if (CALL_SCHEDULER) {
			if (eventList.size() > 0 && eventList[0]->timeStamp == currentTime)
				continue; //process next event from Event queue
		}
		CALL_SCHEDULER = false;
		if (CURRENT_RUNNING_PROCESS == nullptr) {
			CURRENT_RUNNING_PROCESS = scheduler->getNextProcess();
			if (CURRENT_RUNNING_PROCESS == nullptr) {
				continue;
			}
			else {
				// create event to make process runnable for same time.
				Event* newEvt = new Event(currentTime, CURRENT_RUNNING_PROCESS, ProcessState::READY, ProcessState::RUNNING);
				eventList.push_back(newEvt);
				stable_sort(eventList.begin(), eventList.end(), compareTwoEvent);
				printLog(nullptr, newEvt);
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
		if (evt != nullptr) {
			evt->printInfo();
		}
	}
	if (eFlag) {
		if (newEvt != nullptr) {
			cout << "   AddEvent(" << newEvt->timeStamp << ":" << newEvt->process->pid << ":" << stateToString(newEvt->newState) << ")";
			cout << "====>";
			printEventQueue();
		}
	}
	if (tFlag) {
		if (evt != nullptr) {
			cout << "SHCED(" << scheduler->readyQueue.size() << "): ";
			int readyQueueSize = scheduler->readyQueue.size();
			for (int i = 0; i < readyQueueSize; i++) {
				cout << " " << scheduler->readyQueue[i]->pid << ":" << evt->timeStamp;
			}
			cout << endl;
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


//double TotalSimTime;
//double CpuUtil;
//double IOUtil;
//double AvgTurnAround;
//double AvgWaitTime;
//double Throughoput;

void printResult() {
	if (strcmp(sValue, "F") == 0) {
		cout << "FCFS" << endl;
	}
	for (int i = 0; i < processList.size(); i++) {
		Process* proc = processList[i];
		CpuUtil += proc->totalCpuTime;
		AvgTurnAround += (double)proc->finishAtTime - (double)proc->arriveTime;
		AvgWaitTime += (double)proc->cpuWaitingTime;
		TotalSimTime = proc->finishAtTime > TotalSimTime ? proc->finishAtTime : TotalSimTime;


		cout << setfill('0') << setw(4) << i << ":" << setfill(' ') << setw(5) << proc->arriveTime << setw(5) << proc->totalCpuTime << setw(5) << proc->maxCpuBurst << setw(5) << proc->maxIOBurst << setw(2) << proc->staticPrio << " | ";
		cout << setfill(' ') << setw(5) << proc->finishAtTime << setw(6) << proc->finishAtTime - proc->arriveTime << setw(6) << proc->IOTime << setw(6) << proc->cpuWaitingTime << endl;
	}
	CpuUtil = CpuUtil / TotalSimTime * (double)100;
	IOUtil = IOUtil / TotalSimTime * (double)100;
	AvgTurnAround /= processList.size();
	AvgWaitTime /= processList.size();
	Throughoput = (double)processList.size() / TotalSimTime * (double)100;
	printf("SUM: %d %.2lf %.2lf %.2lf %.2lf %.3lf\n", TotalSimTime, CpuUtil, IOUtil, AvgTurnAround, AvgWaitTime, Throughoput);
	//cout << fixed << setprecision(2) << "SUM: " << TotalSimTime << " " << CpuUtil << " " << IOUtil << " " << AvgTurnAround << " " << AvgWaitTime << " " << Throughoput;
}