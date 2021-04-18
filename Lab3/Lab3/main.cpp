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
#include <climits>
#include <iostream>
#include <regex>
#include "BaseObjects.h"
#ifdef _WIN32
#include "getopt.h"
#else
#include <unistd.h>
#include <getopt.h>
#endif

int readOneRandomInt(int brust);
void readAllProcess();
void readNextInstrction();
FrameTableEntry* allocateFrameFromFreeList();
FrameTableEntry* getFrame();

deque<Process*> processList;
int randCount = 0;
int FRAME_COUNT;
const char* inputFile = NULL;
const char* randFile = NULL;
std::ifstream inInputFile;
std::ifstream inRandFile;
string strLine;
char currentInstType;
int currentInstNum;
bool readable = true;
FrameTableEntry** frameTable;
deque<int> freeFrameList;
Pager* thePager;
Process* currentProcess;
vector<PageTabelEntry*>* currentPageTable;

int main(int argc, char** argv) {
	inputFile = "G:\\NYU\\OS\\Labs\\Lab3\\lab3_assign\\inputs\\in10";
	randFile = "G:\\NYU\\OS\\Labs\\Lab3\\lab3_assign\\inputs\\rfile";
	inInputFile.open(inputFile);
	inRandFile.open(randFile);
	inRandFile >> randCount;
	FRAME_COUNT = 32;
	thePager = new FIFO();
	//cout << "MyStruct size:\t" << sizeof(PageTabelEntry) << endl;
	//return 0;
	readAllProcess();
	//init frame table and free list
	frameTable = new FrameTableEntry * [FRAME_COUNT];
	for (int i = 0; i < FRAME_COUNT; i++) {
		frameTable[i] = new FrameTableEntry();
		frameTable[i]->isOccupied = 0;
		frameTable[i]->index = i;
		freeFrameList.push_back(i);
	}
	//for (int i = 0; i < processList.size(); i++)
	//{
	//	processList.at(i)->printProcess();
	//}

	//skip # line before instruction
	while ((strLine == "" && !inInputFile.eof())) {
		getline(inInputFile, strLine);
	}
	int instCount = -1;

	//simulation
	while (true) {
		readNextInstrction();
		if (!readable) {
			break;
		}
		//logic for each instruction 
		//cout << currentInstType << " " << currentInstNum<<endl;
		instCount++;
		cout << instCount << ":" << " ==> " << currentInstType << " " << currentInstNum << endl;
		if (currentInstType == 'c') {
			//switch in process
			currentProcess = processList.at(currentInstNum);
			currentPageTable = &(currentProcess->pageTable);
		}
		else if (currentInstType == 'e') {
			cout << "EXIT current process " << currentProcess->pid<<endl;
			//switch out process
			for (int i = 0; i < currentPageTable->size(); i++) {
				PageTabelEntry* entry = currentPageTable->at(i);
				if (entry->present == 1) {
					freeFrameList.push_back(entry->frameNumber);
					frameTable[entry->frameNumber]->isOccupied = 0;
					cout << " UNMAP " << currentProcess->pid << ":" << i << endl;
					if (entry->modified == 1)
					{
						if (entry->fileMapped == 1) {
							cout << " FOUT" << endl;
						}
						entry->modified = 0;
					}
				}

			}
		}
		else {
			PageTabelEntry* pageTableEntry = currentPageTable->at(currentInstNum);
			if (pageTableEntry->present != 1) {
				//page not in frame, page fault
				if (pageTableEntry->initialized == 0) {
					//not initialized yet, check VMAs
					for (int i = 0; i < currentProcess->virtualMemoryAreas.size(); i++) {
						if (currentProcess->virtualMemoryAreas.at(i)[0] <= currentInstNum
							&& currentProcess->virtualMemoryAreas.at(i)[1] >= currentInstNum) {
							// currentInstNum fall in a VMA
							pageTableEntry->writeProtect = currentProcess->virtualMemoryAreas.at(i)[2];
							pageTableEntry->fileMapped = currentProcess->virtualMemoryAreas.at(i)[3];
							pageTableEntry->notInVMAs = 0;
							pageTableEntry->initialized = 1;
							break;
						}
					}
					if (pageTableEntry->initialized == 0) {
						// currentInstNum fall in a hole
						pageTableEntry->notInVMAs = 1;
						pageTableEntry->initialized = 1;
					}
				}
				if (pageTableEntry->notInVMAs == 1) {
					// currentInstNum fall in a hole
					cout << " SEGV" << endl;
					continue;
				}
				//vaild page in VMA, page in it
				FrameTableEntry* newFrame = getFrame();
				if (newFrame != nullptr) {
					if (newFrame->isOccupied != 0) {
						// frame already occupied
						Process* reverseProcess = processList.at(newFrame->reverseProcessNum);
						PageTabelEntry* reversePageTableEntry = reverseProcess->pageTable.at(newFrame->reverseVirtualTableNum);
						reversePageTableEntry->present = 0;
						cout << " UNMAP " << reverseProcess->pid << ":" << newFrame->reverseVirtualTableNum << endl;
						if (reversePageTableEntry->modified == 1)
						{
							if (reversePageTableEntry->fileMapped == 1) {
								cout << " FOUT" << endl;
							}
							else {
								cout << " OUT" << endl;
							}
							reversePageTableEntry->pageout = 1;
							reversePageTableEntry->modified = 0;
						}
					}
					// page in
					PageTabelEntry* currentPageTableEntry = currentPageTable->at(currentInstNum);
					currentPageTableEntry->present = 1;
					currentPageTableEntry->frameNumber = newFrame->index;
					newFrame->reverseProcessNum = currentProcess->pid;
					newFrame->reverseVirtualTableNum = currentInstNum;
					newFrame->isOccupied = 1;
					if (currentPageTableEntry->fileMapped == 1) {
						cout << " FIN" << endl;
					}
					else if (currentPageTableEntry->pageout == 1) {
						cout << " IN" << endl;
					}
					else {
						cout << " ZERO" << endl;
					}

					cout << " MAP " << newFrame->index << endl;
				}
				else {
					cout << "selected a null frame, should never be here";
				}
			}
			// current page should be loadded already
			PageTabelEntry* currentPageTableEntry = currentPageTable->at(currentInstNum);
			if (currentInstType == 'r') {
				currentPageTableEntry->reference = 1;
			}
			else if (currentInstType == 'w') {
				if (currentPageTableEntry->writeProtect == 1){
					cout << " SEGPROT" << endl;
				}
				else {
					currentPageTableEntry->reference = 1;
					currentPageTableEntry->modified = 1;
				}
			}
		}
	}
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
	//skip first few # lines and get process count
	strLine = "";
	while ((strLine == "" && !inInputFile.eof()) || strLine.rfind("#", 0) == 0) {
		getline(inInputFile, strLine);
	}
	int processNum = stoi(strLine);
	strLine = "";
	int a, b, c, d;
	for (int i = 0; i < processNum; i++) {
		Process* process = new Process();
		while ((strLine == "" && !inInputFile.eof()) || strLine.rfind("#", 0) == 0) {
			getline(inInputFile, strLine);
		}
		int areaNum = stoi(strLine);
		strLine = "";
		for (int i = 0; i < areaNum; i++) {
			inInputFile >> a >> b >> c >> d;
			process->addOneVirtualMemoryArea(a, b, c, d);
		}
		processList.push_back(process);
	}
}

void readNextInstrction() {
	if (!inInputFile.eof()) {
		inInputFile >> currentInstType >> currentInstNum;
	}
	if (currentInstType == '#') {
		readable = false;
	}
}

FrameTableEntry* getFrame() {
	FrameTableEntry* frame = allocateFrameFromFreeList();
	if (frame == nullptr) {
		frame = thePager->selectVictimFrame(frameTable, FRAME_COUNT);
	}
	return frame;
}

FrameTableEntry* allocateFrameFromFreeList() {
	if (freeFrameList.size() > 0) {
		FrameTableEntry* entry = frameTable[freeFrameList.front()];
		freeFrameList.pop_front();
		return entry;
	}
	//empty freeFrameList
	return nullptr;
}