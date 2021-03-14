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
	cout << timeStamp << " <" << process->pid << "> " << timeStamp - process->timeLastStateStart
		<< stateToString(oldState) << " -> " << stateToString(newState) << " ";
	if (newState == ProcessState::RUNNING) {
		cout << "cb=" << process->currentCpuBrust << " rem=" << process->remainingCpuTime << " prio=" << process->dynamicPrio;
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

void FIFO::addProcess(Process* process)
{
}

Process* FIFO::getNextProcess()
{
	return nullptr;
}

void FIFO::test_preempt(Process* p, int curtime) {
}