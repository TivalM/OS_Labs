#pragma once
#include <iostream>
#include <deque>
using namespace std;

enum class ProcessState {
	CREATED,
	READY,
	RUNNING,
	BLOCKED
};

enum class TransitionType {
	CREATE_TO_READY,		//1
	READY_TO_RUNNING,		//2
	RUNNING_TO_BLOCK,		//3
	BLOCKED_TO_READY,
	RUNNING_TO_READY,
	TRANS_TO_PREEMPT	//
};

inline const char* stateToString(ProcessState s) {
	switch (s) {
	case ProcessState::CREATED:	return "CREATED";
	case ProcessState::READY:   return "READY";
	case ProcessState::RUNNING: return "RUNNING";
	case ProcessState::BLOCKED:	return "BLOCKED";
	default:      return "[Unknown ProcessState]";
	}
}

class Process {
	static int counter;
public:
	Process(int arriveTime, int totalCpuTime, int maxCpuBurst, int maxIOBurst, int staticPrio);
	~Process() {}
	void printInfo();

	ProcessState state;
	int pid;
	int arriveTime;
	int totalCpuTime;
	int maxCpuBurst;
	int maxIOBurst;
	int currentCpuBrust = 0;
	int currentIOBrust = 0;
	int remainingCpuTime;
	int staticPrio;
	int dynamicPrio;
	int timeLastStateStart = 0;
	bool expired = false;

	int finishAtTime = 0;
	int turnAroundTime = 0;
	int IOTime = 0;
	int cpuWaitingTime = 0;
};

class Event
{
public:
	Event(int timeStamp, Process* process, ProcessState oldState, ProcessState newState);
	~Event();
	TransitionType getTransitionType();
	void printInfo();


	int timeStamp;
	Process* process;
	ProcessState oldState;
	ProcessState newState;
};

class Scheduler {
public:
	virtual void addProcess(Process* process) = 0;
	virtual Process* getNextProcess() = 0;

	deque<Process*> readyQueue;
};


class FCFS :public Scheduler {
public:
	void addProcess(Process* process);
	Process* getNextProcess();
};

class LCFS :public Scheduler {
public:
	void addProcess(Process* process);
	Process* getNextProcess();
};

class SRTF :public Scheduler {
public:
	void addProcess(Process* process);
	Process* getNextProcess();
};

class PRIO :public Scheduler {
public:
	PRIO(int maxPrio);
	void addProcess(Process* process);
	Process* getNextProcess();

	bool swaped = false;
	deque<deque<Process*>>* mutiLevelReadyQueue;
	deque<deque<Process*>>* mutiLevelExpriedQueue;
};

class PREPRIO :public PRIO {
public:
	PREPRIO(int maxPrio);
	void addProcess(Process* process);
	Process* getNextProcess();

	bool swaped = false;
};