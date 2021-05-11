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

ioRequestEntry* FIFO::getNextIO(deque<ioRequestEntry*>* addRequestList, deque<ioRequestEntry*>* activeRequestList)
{
	ioRequestEntry* request = activeRequestList->at(0);
	activeRequestList->pop_front();
	return request;
}

void FIFO::increaseTrack() {

}

ioRequestEntry* SSTF::getNextIO(deque<ioRequestEntry*>* addRequestList, deque<ioRequestEntry*>* activeRequestList)
{
	return nullptr;
}

void SSTF::increaseTrack()
{
}

ioRequestEntry* LOOK::getNextIO(deque<ioRequestEntry*>* addRequestList, deque<ioRequestEntry*>* activeRequestList)
{
	return nullptr;
}

void LOOK::increaseTrack()
{
}

ioRequestEntry* CLOOK::getNextIO(deque<ioRequestEntry*>* addRequestList, deque<ioRequestEntry*>* activeRequestList)
{
	return nullptr;
}

void CLOOK::increaseTrack()
{
}

ioRequestEntry* FLOOK::getNextIO(deque<ioRequestEntry*>* addRequestList, deque<ioRequestEntry*>* activeRequestList)
{
	return nullptr;
}

void FLOOK::increaseTrack()
{
}
