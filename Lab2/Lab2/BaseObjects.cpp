#include "BaseObjects.h"

int Process::counter = 0;

Process::Process(int arriveTime, int totalCpuTime, int maxCpuBurst, int maxIOBurst, int staticPrio)
{
	pid = counter++;
	state = ProcessState::CREATED;
	this->arriveTime = arriveTime;
	this->totalCpuTime = totalCpuTime;
	this->maxCpuBurst = maxCpuBurst;
	this->maxIOBurst = maxIOBurst;
	this->staticPrio = staticPrio;
	this->dynamicPrio = staticPrio - 1;
	this->remainingCpuTime = totalCpuTime;
}

void Process::printInfo()
{
	cout << stateToString(state) << "<" << pid << "> " << this->arriveTime << " " << this->totalCpuTime << " "
		<< this->maxCpuBurst << " " << this->maxIOBurst << " " << this->staticPrio << endl;
}

Event::Event(int timeStamp, Process* process, ProcessState oldState, ProcessState newState)
{
	this->timeStamp = timeStamp;
	this->process = process;
	this->oldState = oldState;
	this->newState = newState;
}

Event::~Event() {}

void Event::printInfo()
{
	if (oldState != ProcessState::CREATED) {
		cout << timeStamp << "<" << process->pid << ">" << timeStamp - process->timeLastStateStart << " ";
	}
	else {
		cout << timeStamp << "<" << process->pid << ">" << 0 << " ";
	}
	if (process->remainingCpuTime == 0) {
		cout << "Done" << endl;
		return;
	}
	else {
		cout << stateToString(oldState) << " -> " << stateToString(newState) << " ";
	}

	if (newState == ProcessState::RUNNING || (newState == ProcessState::READY && oldState == ProcessState::RUNNING)) {
		cout << "cb=" << process->currentCpuBrust << " rem=" << process->remainingCpuTime << " prio=" << process->dynamicPrio << endl;
	}
	else if (newState == ProcessState::BLOCKED || (newState == ProcessState::READY && oldState == ProcessState::BLOCKED)) {
		cout << "ib=" << process->currentIOBrust << " rem=" << process->remainingCpuTime << endl;
	}
	else {
		cout << endl;
	}
}

TransitionType Event::getTransitionType()
{
	if (oldState == ProcessState::CREATED) {
		return TransitionType::CREATE_TO_READY;
	}
	else if (oldState == ProcessState::READY) {
		if (newState == ProcessState::RUNNING) {
			return TransitionType::READY_TO_RUNNING;
		}
	}
	else if (oldState == ProcessState::RUNNING) {
		if (newState == ProcessState::READY) {
			return TransitionType::RUNNING_TO_READY;
		}
		else if (newState == ProcessState::BLOCKED) {
			return TransitionType::RUNNING_TO_BLOCK;
		}
	}
	else if (oldState == ProcessState::BLOCKED) {
		if (newState == ProcessState::READY) {
			return TransitionType::BLOCKED_TO_READY;
		}
	}
}

void FCFS::addProcess(Process* process) {
	readyQueue.push_back(process);
}

Process* FCFS::getNextProcess()
{
	if (readyQueue.size() > 0) {
		if (readyQueue[0]->remainingCpuTime == 0)
		{
			readyQueue.pop_front();
			return getNextProcess();
		}
		else {
			Process* proc = readyQueue[0];
			readyQueue.pop_front();
			return proc;
		}
	}
	else {
		return nullptr;
	}
}


void LCFS::addProcess(Process* process)
{
	readyQueue.push_front(process);
}

Process* LCFS::getNextProcess()
{
	if (readyQueue.size() > 0) {
		if (readyQueue[0]->remainingCpuTime == 0)
		{
			readyQueue.pop_front();
			return getNextProcess();
		}
		else {
			Process* proc = readyQueue[0];
			readyQueue.pop_front();
			return proc;
		}
	}
	else {
		return nullptr;
	}
}

void SRTF::addProcess(Process* process)
{
	readyQueue.push_back(process);
}

Process* SRTF::getNextProcess()
{
	if (readyQueue.size() > 0) {
		int index = 0;
		int timeRemain = readyQueue[0]->remainingCpuTime;
		for (int i = 0; i < readyQueue.size(); i++)
		{
			if (readyQueue[i]->remainingCpuTime < timeRemain && readyQueue[i]->remainingCpuTime != 0) {
				timeRemain = readyQueue[i]->remainingCpuTime;
				index = i;
			}
		}
		Process* proc = readyQueue[index];
		readyQueue.erase(readyQueue.begin() + index);
		return proc;
	}
	else {
		return nullptr;
	}
}

PRIO::PRIO(int maxPrio)
{
	mutiLevelReadyQueue = new deque<deque<Process*>>();
	mutiLevelExpriedQueue = new deque<deque<Process*>>();

	//priority levels [0..maxprio-1]
	for (int i = 0; i < maxPrio; i++) {
		mutiLevelReadyQueue->push_back(deque<Process*>());
		mutiLevelExpriedQueue->push_back(deque<Process*>());
	}
}

void PRIO::addProcess(Process* process)
{
	if (process->expired)
	{
		//add into expried queue
		mutiLevelExpriedQueue->at(process->dynamicPrio).push_back(process);
		process->expired = false;
	}
	else {
		mutiLevelReadyQueue->at(process->dynamicPrio).push_back(process);
	}
}

Process* PRIO::getNextProcess()
{
	for (int i = mutiLevelReadyQueue->size() - 1; i >= 0; i--) {
		while (mutiLevelReadyQueue->at(i).size() != 0) {
			if (mutiLevelReadyQueue->at(i)[0]->remainingCpuTime == 0) {
				readyQueue.pop_front();
			}
			else {
				Process* proc = mutiLevelReadyQueue->at(i)[0];
				mutiLevelReadyQueue->at(i).pop_front();
				swaped = false;
				return proc;
			}
		}
	}
	if (!swaped) {
		//empty active queue
		deque<deque<Process*>>* tmp = mutiLevelReadyQueue;
		mutiLevelReadyQueue = mutiLevelExpriedQueue;
		mutiLevelExpriedQueue = tmp;
		swaped = true;
		return getNextProcess();
	}
	swaped = false;
	return nullptr;
}