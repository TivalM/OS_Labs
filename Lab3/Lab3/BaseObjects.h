#pragma once
#include <iostream>
#include <vector>
#include <deque>
#include <fstream>
using namespace std;


static const int PAGE_TABLE_ENTRY_NUM = 64;

struct PageTabelEntry {
    unsigned initialized : 1;
    unsigned present : 1;
    unsigned notInVMAs : 1;
    unsigned reference : 1;
    unsigned modified : 1;
    unsigned writeProtect : 1;
    unsigned pageout : 1;
    unsigned fileMapped : 1;
    unsigned int frameNumber : 7;           //0-127
    unsigned reservedBits : 10;
};

struct FrameTableEntry {
    unsigned isOccupied : 1;
    unsigned reverseVirtualTableNum : 6;    //0-63
    unsigned reverseProcessNum : 4;         //max 10 process
    unsigned long lastUsedTime;
    unsigned int age;
};

class Process
{
    static int counter;
public:
    Process();
    ~Process();
    void addOneVirtualMemoryArea(int startPage, int endingPage, int writeProtected, int fileMapped);
    void printVirtualMemoryAreas();
    void printPageTable();
    void clearPageTable();
    void printProcessSummary();
    int pid;
    unsigned long unmaps;
    unsigned long maps;
    unsigned long ins;
    unsigned long outs;
    unsigned long fins;
    unsigned long fouts;
    unsigned long zeros;
    unsigned long segv;
    unsigned long segprot;
    vector<int*> virtualMemoryAreas;
    PageTabelEntry pageTable[PAGE_TABLE_ENTRY_NUM];
};

class Pager
{
public:
    virtual FrameTableEntry* selectVictimFrame(unsigned long currentInst, deque<Process*>& processes, FrameTableEntry* frameTable, int frameTableSize) = 0;
};

class FIFO : public Pager {
public:
    int hand = 0;
    FrameTableEntry* selectVictimFrame(unsigned long currentInst, deque<Process*>& processes, FrameTableEntry* frameTable, int frameTableSize);
};

class CLOCK : public FIFO {
public:
    int hand = 0;
    FrameTableEntry* selectVictimFrame(unsigned long currentInst, deque<Process*>& processes, FrameTableEntry* frameTable, int frameTableSize);
};

class RANDOM : public Pager {
public:
    RANDOM(const char* randFile);
    FrameTableEntry* selectVictimFrame(unsigned long currentInst, deque<Process*>& processes, FrameTableEntry* frameTable, int frameTableSize);
    int readOneRandomInt(int seed);
    const char* randFile;
    int randCount;
    std::ifstream inRandFile;
};

class NRU : public Pager {
public:
    int hand = 0;
    unsigned long lastCalledInst = -1;
    FrameTableEntry* selectVictimFrame(unsigned long currentInst, deque<Process*>& processes, FrameTableEntry* frameTable, int frameTableSize);
};

class AGING : public Pager {
public:
    int hand = 0;
    FrameTableEntry* selectVictimFrame(unsigned long currentInst, deque<Process*>& processes, FrameTableEntry* frameTable, int frameTableSize);
};

class WORKINGSET : public Pager {
public:
    int hand = 0;
    FrameTableEntry* selectVictimFrame(unsigned long currentInst, deque<Process*>& processes, FrameTableEntry* frameTable, int frameTableSize);
};