#pragma once
#include <iostream>
#include <vector>
#include <deque>
#include <fstream>
#include "BaseObjects.h"
using namespace std;

ioRequestEntry::ioRequestEntry(int issueTime, int track) {
	this->issueTime = issueTime;
	this->track = track;
	this->startTime = -1;
	this->endTime = -1;
}

void ioRequestEntry::printBasicInfo() {
	cout << issueTime << " " << track << endl;
}

ioRequestEntry* FIFO::getNextIO(unsigned int trackPointer)
{
	ioRequestEntry* request = activeRequestList->at(0);
	activeRequestList->pop_front();
	return request;
}

void FIFO::increaseTrack() {

}

ioRequestEntry* SSTF::getNextIO(unsigned int trackPointer)
{
	int distance = INT_MAX;
	int selectedIndex = 0;
	for (int i = 0; i < activeRequestList->size(); i++) {
		int newDistance = labs(activeRequestList->at(i)->track - trackPointer);
		if (newDistance < distance){
			selectedIndex = i;
			distance = newDistance;
		}
	}
	ioRequestEntry* request = activeRequestList->at(selectedIndex);
	activeRequestList->erase(activeRequestList->begin() + selectedIndex);
	return request;
}

void SSTF::increaseTrack()
{
}

ioRequestEntry* LOOK::getNextIO(unsigned int trackPointer)
{
	return nullptr;
}

void LOOK::increaseTrack()
{
}

ioRequestEntry* CLOOK::getNextIO(unsigned int trackPointer)
{
	return nullptr;
}

void CLOOK::increaseTrack()
{
}

ioRequestEntry* FLOOK::getNextIO(unsigned int trackPointer)
{
	return nullptr;
}

void FLOOK::increaseTrack()
{
}

Scheduler::Scheduler(){
}
