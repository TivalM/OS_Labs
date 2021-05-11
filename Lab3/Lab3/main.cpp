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

int FRAME_COUNT;
unsigned long INST_COUNT = -1;
unsigned long CTX_SWITCHES = 0;
unsigned long PROCESS_EXITS = 0;
unsigned long long COST = 0;
char type;

int time_rw = 1;
int time_ctx = 130;
int time_exit = 1250;
int time_maps = 300;
int time_unmaps = 400;
int time_ins = 3100;
int time_outs = 2700;
int time_fins = 2800;
int time_fouts = 2400;
int time_zeros = 140;
int time_segv = 340;
int time_segprot = 420;

const char* inputFile = NULL;
const char* randFile = NULL;
std::ifstream inInputFile;
string strLine;
char currentInstType;
int currentInstNum;
bool readable = true;
FrameTableEntry frameTable[128];
deque<int> freeFrameList;
Pager* thePager;
Process* currentProcess;
PageTabelEntry* currentPageTable;

bool FlagO = false;
bool FlagP = false;
bool FlagF = false;
bool FlagS = false;

#define frame_idx(frame) ((frame)-frameTable)

int main(int argc, char** argv) {
	int c;
	while ((c = getopt(argc, argv, "f:a:o:")) != -1)
	{
		switch (c) {
		case 'f':
			FRAME_COUNT = atoi(optarg);
			break;
		case 'a':
			type = optarg[0];
			break;
		case 'o':
			string value = optarg;
			if (value.find("O") != string::npos){
				FlagO = true;
			}
			if(value.find("P") != string::npos){
				FlagP = true;
			}
			if (value.find("F") != string::npos) {
				FlagF = true;
			}
			if (value.find("S") != string::npos) {
				FlagS = true;
			}
			break;
		}
	}

	inputFile = argv[optind];
	randFile = argv[optind + 1];
	inInputFile.open(inputFile);

	if (type == 'f') {
		thePager = new FIFO();
	}
	else if (type == 'r') {
		thePager = new RANDOM(randFile);
	}
	else if (type == 'c') {
		thePager = new CLOCK();
	}
	else if (type == 'e') {
		thePager = new NRU();
	}
	else if (type == 'a') {
		thePager = new AGING();
	}
	else if (type == 'w') {
		thePager = new WORKINGSET();
	}

	//cout << "MyStruct size:\t" << sizeof(PageTabelEntry) << endl;
	//return 0;
	readAllProcess();
	//init frame table and free list
	for (int i = 0; i < FRAME_COUNT; i++) {
		frameTable[i].isOccupied = 0;
		frameTable[i].age = 0;
		frameTable[i].lastUsedTime = 0;
		freeFrameList.push_back(i);
	}
	//for (int i = 0; i < processList.size(); i++)
	//{
	//	processList.at(i)->printVirtualMemoryAreas();
	//}

	//skip line before instruction
	while ((strLine == "" && !inInputFile.eof())) {
		getline(inInputFile, strLine);
	}

	//simulation
	while (true) {
		readNextInstrction();
		if (!readable) {
			break;
		}
		//logic for each instruction 
		//cout << currentInstType << " " << currentInstNum<<endl;
		INST_COUNT++;
		if (FlagO){
			cout << INST_COUNT << ":" << " ==> " << currentInstType << " " << currentInstNum << endl;
		}
		if (currentInstType == 'c') {
			//switch in process
			CTX_SWITCHES++;
			COST += time_ctx;
			currentProcess = processList.at(currentInstNum);
			currentPageTable = currentProcess->pageTable;
		}
		else if (currentInstType == 'e') {
			if (FlagO) {
				cout << "EXIT current process " << currentProcess->pid << endl;
			}
			//switch out process
			PROCESS_EXITS++;
			COST += time_exit;
			for (int i = 0; i < PAGE_TABLE_ENTRY_NUM; i++) {
				PageTabelEntry* entry = &currentPageTable[i];
				if (entry->present == 1) {
					freeFrameList.push_back(entry->frameNumber);
					frameTable[entry->frameNumber].isOccupied = 0;
					frameTable[entry->frameNumber].age = 0;
					frameTable[entry->frameNumber].lastUsedTime = 0;
					if (FlagO) {
						cout << " UNMAP " << currentProcess->pid << ":" << i << endl;
					}
					currentProcess->unmaps++;
					COST += time_unmaps;
					if (entry->modified == 1)
					{
						if (entry->fileMapped == 1) {
							if (FlagO) {
								cout << " FOUT" << endl;
							}
							currentProcess->fouts++;
							COST += time_fouts;
						}
					}
				}

			}
			currentProcess->clearPageTable();
		}
		else {
			PageTabelEntry* pageTableEntry = &currentPageTable[currentInstNum];
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
					if (FlagO) {
						cout << " SEGV" << endl;
					}
					currentProcess->segv++;
					COST += time_segv;
					if (currentInstType == 'r' || currentInstType == 'w') {
						COST += time_rw;
					}
					continue;
				}
				//vaild page in VMA, page in it
				FrameTableEntry* newFrame = getFrame();
				if (newFrame != nullptr) {
					if (newFrame->isOccupied != 0) {
						// frame already occupied
						Process* reverseProcess = processList.at(newFrame->reverseProcessNum);
						PageTabelEntry* reversePageTableEntry = &reverseProcess->pageTable[newFrame->reverseVirtualTableNum];
						reversePageTableEntry->present = 0;
						if (FlagO) {
							cout << " UNMAP " << reverseProcess->pid << ":" << newFrame->reverseVirtualTableNum << endl;
						}
						newFrame->isOccupied = 0;
						reverseProcess->unmaps++;
						COST += time_unmaps;
						if (reversePageTableEntry->modified == 1)
						{
							if (reversePageTableEntry->fileMapped == 1) {
								if (FlagO) {
									cout << " FOUT" << endl;
								}
								reverseProcess->fouts++;
								COST += time_fouts;
							}
							else {
								if (FlagO) {
									cout << " OUT" << endl;
								}
								reverseProcess->outs++;
								reversePageTableEntry->pageout = 1;
								COST += time_outs;
							}
							reversePageTableEntry->modified = 0;
							reversePageTableEntry->reference = 0;
						}
					}
					// page in
					PageTabelEntry* currentPageTableEntry = &currentPageTable[currentInstNum];
					currentPageTableEntry->present = 1;
					currentPageTableEntry->frameNumber = frame_idx(newFrame);
					newFrame->reverseProcessNum = currentProcess->pid;
					newFrame->reverseVirtualTableNum = currentInstNum;
					newFrame->isOccupied = 1;
					newFrame->age = 0;
					newFrame->lastUsedTime = INST_COUNT;
					if (currentPageTableEntry->fileMapped == 1) {
						if (FlagO) {
							cout << " FIN" << endl;
						}
						currentProcess->fins++;
						COST += time_fins;
					}
					else if (currentPageTableEntry->pageout == 1) {
						if (FlagO) {
							cout << " IN" << endl;
						}
						currentProcess->ins++;
						COST += time_ins;
					}
					else {
						if (FlagO) {
							cout << " ZERO" << endl;
						}
						currentProcess->zeros++;
						COST += time_zeros;
					}
					currentPageTableEntry->modified = 0;
					currentPageTableEntry->reference = 0;
					if (FlagO) {
						cout << " MAP " << frame_idx(newFrame) << endl;
					}
					currentProcess->maps++;
					COST += time_maps;
				}
				else {
					cout << "selected a null frame, should never be here";
				}
			}
			// current page should be loadded already
			PageTabelEntry* currentPageTableEntry = &currentPageTable[currentInstNum];
			if (currentInstType == 'r') {
				currentPageTableEntry->reference = 1;
				COST += time_rw;
			}
			else if (currentInstType == 'w') {
				COST += time_rw;
				currentPageTableEntry->reference = 1;
				if (currentPageTableEntry->writeProtect == 1) {
					if (FlagO) {
						cout << " SEGPROT" << endl;
					}
					currentProcess->segprot++;
					COST += time_segprot;
				}
				else {
					currentPageTableEntry->modified = 1;
				}
			}
		}
	}

	//print each PT
	if (FlagP) {
		for (int i = 0; i < processList.size(); i++) {
			processList.at(i)->printPageTable();
		}
	}
	//print FT
	if (FlagF) {
		cout << "FT: ";
		for (int i = 0; i < FRAME_COUNT; i++) {
			if (frameTable[i].isOccupied) {
				cout << frameTable[i].reverseProcessNum << ":" << frameTable[i].reverseVirtualTableNum << " ";
			}
			else {
				cout << "* ";
			}
		}
		cout << endl;
	}
	//print process summary
	if (FlagS) {
		for (int i = 0; i < processList.size(); i++) {
			processList.at(i)->printProcessSummary();
		}
		//print total summary
		printf("TOTALCOST %lu %lu %lu %llu %llu\n",
			INST_COUNT + 1, CTX_SWITCHES, PROCESS_EXITS, COST, sizeof(PageTabelEntry));
	}
}

void readAllProcess() {
	//skip first few # lines and get process count
	strLine = "";
	while ((strLine == "" || strLine.rfind("#", 0) == 0) && !inInputFile.eof()) {
		getline(inInputFile, strLine);
	}
	int processNum = stoi(strLine);
	strLine = "";
	int a, b, c, d;
	for (int i = 0; i < processNum; i++) {
		Process* process = new Process();
		while ((strLine == "" || strLine.rfind("#", 0) == 0) && !inInputFile.eof()) {
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
	currentInstType = '#';
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
		frame = thePager->selectVictimFrame(INST_COUNT, processList, frameTable, FRAME_COUNT);
	}
	return frame;
}

FrameTableEntry* allocateFrameFromFreeList() {
	if (freeFrameList.size() > 0) {
		FrameTableEntry* entry = &frameTable[freeFrameList.front()];
		freeFrameList.pop_front();
		return entry;
	}
	//empty freeFrameList
	return nullptr;
}