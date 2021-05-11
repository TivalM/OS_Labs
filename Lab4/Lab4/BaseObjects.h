#pragma once
#include <iostream>
#include <vector>
#include <deque>
#include <fstream>
using namespace std;


static const int PAGE_TABLE_ENTRY_NUM = 64;

class ioRequestEntry {
public:
    ioRequestEntry(int issueTime, int track);
    int issueTime;
    int track;
    int startTime;
    int endTime;

    void printBasicInfo();
};

class Scheduler
{
public:
    Scheduler();
    deque<ioRequestEntry*>* activeRequestList = new deque<ioRequestEntry*>();
    virtual ioRequestEntry* getNextIO(unsigned int trackPointer) = 0;
    virtual void increaseTrack() = 0;
};

class FIFO : public Scheduler {
public:
    ioRequestEntry* getNextIO(unsigned int trackPointer);
    void increaseTrack();
};

class SSTF :public Scheduler {
public:
	ioRequestEntry* getNextIO(unsigned int trackPointer);
    void increaseTrack();
};

class LOOK :public Scheduler {
public:
    bool increasePointer = true;
	ioRequestEntry* getNextIO(unsigned int trackPointer);
    void increaseTrack();
};

class CLOOK :public Scheduler {
public:
	ioRequestEntry* getNextIO(unsigned int trackPointer);
    void increaseTrack();
};

class FLOOK :public Scheduler {
public:
    ioRequestEntry* getNextIO(unsigned int trackPointer);
    void increaseTrack();
};