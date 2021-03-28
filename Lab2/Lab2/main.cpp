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
#include <getopt.h>
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

bool vFlag = false;
bool tFlag = false;
bool eFlag = false;
const char* sValue;
char type;
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
	// proper way to parse arguments
	int c;
	while ((c = getopt(argc, argv, "vtes:")) != -1)
	{
		switch (c) {
		case 'v':
			vFlag = true;
			break;
		case 't':
			tFlag = true;
			break;
		case 'e':
			eFlag = true;
			break;
		case 's':
			sValue = optarg;
			break;
		}
	}
	inputFile = argv[optind];
	randFile = argv[optind + 1];
	if (sValue[0] != 'F' && sValue[0] != 'L' && sValue[0] != 'S') {
		//prase -s
		string s = sValue;
		s = s.substr(1, s.size());
		string delimiter = ":";
		string q = s.substr(0, s.find(delimiter));
		quantumNum = stoi(q);
		if (q.length()!= s.length()){
			s.erase(0, s.find(delimiter) + delimiter.length());
			maxProiNum = stoi(s);
		}
	}
	type = sValue[0];
	if (vFlag) {
		cout << "vte" << vFlag << tFlag << eFlag << " " << type << " " << quantumNum << " " << maxProiNum << " " << inputFile << " " << randFile;
	}
	inInputFile.open(inputFile);
	inRandFile.open(randFile);
	inRandFile >> randCount;


	readAllProcess();

	if (type == 'F') {
		scheduler = new FCFS();
	}
	else if (type == 'L') {
		scheduler = new LCFS();
	}
	else if (type == 'S') {
		scheduler = new SRTF();
	}
	else if (type == 'R') {
		scheduler = new FCFS();
	}
	else if (type == 'P') {
		scheduler = new PRIO(maxProiNum);
	}
	else if (type == 'E') {
		scheduler = new PRIO(maxProiNum);
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
		return readOneRandomInt(seed);
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
			if (type == 'E' && CURRENT_RUNNING_PROCESS != nullptr && CURRENT_RUNNING_PROCESS->dynamicPrio < proc->dynamicPrio) {
				//preempt current process
				//remove all event related to current process
				for (int i = 0; i < eventList.size(); i++) {
					if (eventList[i]->process == CURRENT_RUNNING_PROCESS && eventList[i]->timeStamp != currentTime) {
						eventList.erase(eventList.begin() + i);
						//add an new event to preempt current process
						Event* newEvt = new Event(currentTime, CURRENT_RUNNING_PROCESS, ProcessState::RUNNING, ProcessState::READY);
						eventList.push_back(newEvt);
						stable_sort(eventList.begin(), eventList.end(), compareTwoEvent);
					}
				}
			}
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
			if (type == 'E' && CURRENT_RUNNING_PROCESS != nullptr && CURRENT_RUNNING_PROCESS->dynamicPrio < proc->dynamicPrio) {
				//preempt current process
				//remove all event related to current process
				for (int i = 0; i < eventList.size(); i++) {
					if (eventList[i]->process == CURRENT_RUNNING_PROCESS && eventList[i]->timeStamp != currentTime) {
						eventList.erase(eventList.begin() + i);
						//add an new event to preempt current process
						Event* newEvt = new Event(currentTime, CURRENT_RUNNING_PROCESS, ProcessState::RUNNING, ProcessState::READY);
						eventList.push_back(newEvt);
						stable_sort(eventList.begin(), eventList.end(), compareTwoEvent);
					}
				}
			}
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
			if (proc->currentCpuBrust == 0) {
				//generate a cpu brust
				cb = readOneRandomInt(proc->maxCpuBurst);
				cb = cb > proc->maxCpuBurst ? proc->maxCpuBurst : cb;
				cb = cb > proc->remainingCpuTime ? proc->remainingCpuTime : cb;
				proc->currentCpuBrust = cb;
			}
			else {
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
			proc->currentCpuBrust = 0;
			if (proc->remainingCpuTime == 0) {
				//process should terminate here 
				proc->finishAtTime = currentTime;
				printLog(evt, nullptr);
			}
			else {
				//generate an io brust
				int ib = readOneRandomInt(proc->maxIOBurst);
				ib = ib > proc->maxIOBurst ? proc->maxIOBurst : ib;
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
			CURRENT_RUNNING_PROCESS = nullptr;
			proc->remainingCpuTime -= timeDuration;
			proc->currentCpuBrust -= timeDuration;

			proc->dynamicPrio -= 1;
			if (proc->dynamicPrio == -1) {
				proc->expired = true;
				proc->dynamicPrio = proc->staticPrio - 1;
			}
			scheduler->addProcess(proc);
			CALL_SCHEDULER = true;
			printLog(evt, nullptr);

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
		if (tFlag) {
			if (evt != nullptr) {
				if (type != 'E' && type != 'P')
				{
					cout << "SHCED(" << scheduler->readyQueue.size() << "): ";
					int readyQueueSize = scheduler->readyQueue.size();
					for (int i = 0; i < readyQueueSize; i++) {
						cout << " " << scheduler->readyQueue[i]->pid << ":" << evt->timeStamp;
					}
					cout << endl;
				}
				else
				{
					PRIO* Prio = (PRIO*)scheduler;
					cout << "{";
					for (int i = 0; i < (Prio->mutiLevelReadyQueue->size()); i++) {
						cout << "[";
						for (int j = 0; j < (Prio->mutiLevelReadyQueue->at(i).size()); j++) {
							cout << Prio->mutiLevelReadyQueue->at(i)[j]->pid << ",";
						}
						cout << "]";
					}
					cout << "}  {";
					for (int i = 0; i < (Prio->mutiLevelExpriedQueue->size()); i++) {
						cout << "[";
						for (int j = 0; j < (Prio->mutiLevelExpriedQueue->at(i).size()); j++) {
							cout << Prio->mutiLevelExpriedQueue->at(i)[j]->pid << ",";
						}
						cout << "]";
					}
					cout << "}";
					cout << endl;
				}
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


//double TotalSimTime;
//double CpuUtil;
//double IOUtil;
//double AvgTurnAround;
//double AvgWaitTime;
//double Throughoput;

void printResult() {
	if (type == 'F') {
		cout << "FCFS" << endl;
	}
	else if (type == 'L') {
		cout << "LCFS" << endl;
	}
	else if (type == 'S') {
		cout << "SRTF" << endl;
	}
	else if (type == 'R') {
		cout << "RR " << quantumNum << endl;
	}
	else if (type == 'P') {
		cout << "PRIO " << quantumNum << endl;
	}
	else if (type == 'E') {
		cout << "PREPRIO " << quantumNum << endl;
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