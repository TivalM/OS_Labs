#pragma once
#include <iostream>
#include <vector>
using namespace std;


static int PAGE_TABLE_ENTRY_NUM = 64;

struct PageTabelEntry {
    unsigned valid : 1;
    unsigned peferenced : 1;
    unsigned modified : 1;
    unsigned writeProtect : 1;
    unsigned pageout : 1;
    unsigned frameNumber : 7;
    unsigned reservedBits : 20;
};

class Process
{
    static int counter;
public:
    Process();
    ~Process();
    void addOneVirtualMemoryArea(int startPage, int endingPage, int writeProtected, int fileMapped);
    void printProcess();

    int pid;
    vector<int*> virtualMemoryAreas;
};