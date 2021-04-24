#include "BaseObjects.h"

int Process::counter = 0;

Process::Process()
{
	pid = counter++;
	//for (int i = 0; i < PAGE_TABLE_ENTRY_NUM; i++) {
	//	PageTabelEntry* entry = new PageTabelEntry();
	//	pageTable.push_back(entry);
	//}
}

Process::~Process()
{
	for (int i = 0; i < this->virtualMemoryAreas.size(); i++) {
		delete[] virtualMemoryAreas[i];
		delete[] pageTable;
	}
}

void Process::addOneVirtualMemoryArea(int startPage, int endingPage, int writeProtected, int fileMapped)
{
	int* memoryArea = new int[4]{ startPage, endingPage, writeProtected, fileMapped };
	this->virtualMemoryAreas.push_back(memoryArea);
}

void Process::printVirtualMemoryAreas() {
	cout << "pid: " << this->pid << endl;
	for (int i = 0; i < this->virtualMemoryAreas.size(); i++) {
		cout << this->virtualMemoryAreas.at(i)[0] << " " << this->virtualMemoryAreas.at(i)[1] <<
			" " << this->virtualMemoryAreas.at(i)[2] << " " << this->virtualMemoryAreas.at(i)[3] << endl;
	}
	cout << endl;
}

void Process::printPageTable() {
	cout << "PT[" << pid << "]: ";
	for (int i = 0; i < PAGE_TABLE_ENTRY_NUM; i++) {
		if (pageTable[i].present == 1) {
			cout << i << ":";
			cout << ((pageTable[i].reference == 1) ? "R" : "-");
			cout << ((pageTable[i].modified == 1) ? "M" : "-");
			cout << ((pageTable[i].pageout == 1) ? "S" : "-");
			cout << " ";
		}
		else if (pageTable[i].pageout == 1 && pageTable[i].fileMapped == 0) {
			cout << "# ";
		}
		else {
			cout << "* ";
		}
	}
	cout << endl;
}

void Process::clearPageTable() {
	for (int i = 0; i < PAGE_TABLE_ENTRY_NUM; i++) {
		pageTable[i].initialized = 0;
		pageTable[i].frameNumber = 0;
		pageTable[i].modified = 0;
		pageTable[i].present = 0;
		pageTable[i].reference = 0;
		pageTable[i].pageout = 0;
	}
}

void Process::printProcessSummary() {
	printf("PROC[%d]: U=%lu M=%lu I=%lu O=%lu FI=%lu FO=%lu Z=%lu SV=%lu SP=%lu\n",
		pid, unmaps, maps, ins, outs, fins,
		fouts, zeros, segv, segprot);
}

FrameTableEntry* FIFO::selectVictimFrame(unsigned long currentInst, deque<Process*>& processes, FrameTableEntry* frameTable, int frameTableSize)
{
	//hand [0, frameTableSize)
	if (hand == frameTableSize) {
		hand = 0;
	}
	return &frameTable[hand++];
}

FrameTableEntry* CLOCK::selectVictimFrame(unsigned long currentInst, deque<Process*>& processes, FrameTableEntry* frameTable, int frameTableSize)
{
	//hand [0, frameTableSize)
	if (hand == frameTableSize) {
		hand = 0;
	}
	FrameTableEntry* entry = &frameTable[hand++];
	PageTabelEntry* pageTableEntry = &processes.at(entry->reverseProcessNum)->pageTable[entry->reverseVirtualTableNum];
	if (pageTableEntry->reference == 1) {
		pageTableEntry->reference = 0;
		return selectVictimFrame(currentInst, processes, frameTable, frameTableSize);
	}
	return entry;
}

FrameTableEntry* NRU::selectVictimFrame(unsigned long currentInst, deque<Process*>& processes, FrameTableEntry* frameTable, int frameTableSize)
{
	bool needResetReferenceBit = (currentInst - lastCalledInst >= 50);
	if (needResetReferenceBit) {
		lastCalledInst = currentInst;
	}

	//first frame
	FrameTableEntry* frameEntry = &frameTable[hand];
	PageTabelEntry* pageTableEntry = &processes.at(frameEntry->reverseProcessNum)->pageTable[frameEntry->reverseVirtualTableNum];
	int start = hand;
	unsigned int currentSelectedFrameIndex = hand;
	unsigned int selectedFrameModified = pageTableEntry->modified;
	unsigned int selectedFrameReferenced = pageTableEntry->reference;
	hand = hand + 1 >= frameTableSize ? 0 : hand + 1;
	//if first entry is suitable
	if (pageTableEntry->modified == 0 && pageTableEntry->reference == 0 && !needResetReferenceBit) {
		//hand already increased
		return frameEntry;
	}
	if (needResetReferenceBit) {
		pageTableEntry->reference = 0;
	}

	//try to find a better one than the beginning || reset rest entries
	for (; hand != start; hand++) {
		if (hand == frameTableSize) {
			hand = 0;
			if (hand == start) {
				break;
			}
		}
		//start from start + 1 || 0
		frameEntry = &frameTable[hand];
		pageTableEntry = &processes.at(frameEntry->reverseProcessNum)->pageTable[frameEntry->reverseVirtualTableNum];
		if (pageTableEntry->modified == 0 && pageTableEntry->reference == 0 && !needResetReferenceBit) {
				hand = hand + 1 >= frameTableSize ? 0 : hand + 1;
				return frameEntry;
		}

		if (pageTableEntry->reference < selectedFrameReferenced) {
			//select a better frame
			currentSelectedFrameIndex = hand;
			selectedFrameModified = pageTableEntry->modified;
			selectedFrameReferenced = pageTableEntry->reference;
		}
		else if (pageTableEntry->reference == selectedFrameReferenced) {
			if (pageTableEntry->modified < selectedFrameModified) {
				//select a better frame
				currentSelectedFrameIndex = hand;
				selectedFrameModified = pageTableEntry->modified;
				selectedFrameReferenced = pageTableEntry->reference;
			}
		}
		// reset reference bit
		if (needResetReferenceBit && pageTableEntry->reference != 0) {
			pageTableEntry->reference = 0;
		}
	}
	// scaned all entries
	hand = currentSelectedFrameIndex + 1 >= frameTableSize ? 0 : currentSelectedFrameIndex + 1;
	return &frameTable[currentSelectedFrameIndex];
}

RANDOM::RANDOM(const char* randFile)
{
	this->randFile = randFile;
	inRandFile.open(randFile);
	inRandFile >> randCount;
}

FrameTableEntry* RANDOM::selectVictimFrame(unsigned long currentInst, deque<Process*>& processes, FrameTableEntry* frameTable, int frameTableSize)
{
	return &frameTable[readOneRandomInt(frameTableSize)];
}

int RANDOM::readOneRandomInt(int seed)
{
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
	return randInt % seed;
}

FrameTableEntry* AGING::selectVictimFrame(unsigned long currentInst, deque<Process*>& processes, FrameTableEntry* frameTable, int frameTableSize)
{
	//aging
	for (int i = 0; i < frameTableSize; i++) {
		if (frameTable[i].isOccupied == 1) {
			Process* reverseProcess = processes.at(frameTable[i].reverseProcessNum);
			PageTabelEntry* reversePageTableEntry = &reverseProcess->pageTable[frameTable[i].reverseVirtualTableNum];
			frameTable[i].age = frameTable[i].age >> 1;
			if (reversePageTableEntry->reference == 1) {
				frameTable[i].age = (frameTable[i].age | 0x80000000);
				reversePageTableEntry->reference = 0;
			}
		}
	}

	//basic struct looks like NRU
	int start = hand;
	unsigned int currentSelectedFrameIndex = hand;
	unsigned int  tmpAge = frameTable[hand].age;
	hand = hand + 1 >= frameTableSize ? 0 : hand + 1;

	if (frameTable[currentSelectedFrameIndex].age == 0) {
		return &frameTable[currentSelectedFrameIndex];
	}

	for (; hand != start; hand++) {
		if (hand == frameTableSize) {
			hand = 0;
			if (hand == start) {
				break;
			}
		}
		if (frameTable[hand].age == 0) {
			currentSelectedFrameIndex = hand;
			hand = hand + 1 >= frameTableSize ? 0 : hand + 1;
			return &frameTable[currentSelectedFrameIndex];
		}
		if (frameTable[hand].age < tmpAge) {
			// select a better entry
			currentSelectedFrameIndex = hand;
			tmpAge = frameTable[hand].age;
		}
	}
	hand = currentSelectedFrameIndex + 1 >= frameTableSize ? 0 : currentSelectedFrameIndex + 1;
	return &frameTable[currentSelectedFrameIndex];
}

FrameTableEntry* WORKINGSET::selectVictimFrame(unsigned long currentInst, deque<Process*>& processes, FrameTableEntry* frameTable, int frameTableSize)
{
	//basic struct looks like NRU
	int start = hand;
	unsigned int currentSelectedFrameIndex = hand;
	hand = hand + 1 >= frameTableSize ? 0 : hand + 1;

	FrameTableEntry* frameEntry = &frameTable[currentSelectedFrameIndex];
	PageTabelEntry* pageTableEntry = &processes.at(frameEntry->reverseProcessNum)->pageTable[frameEntry->reverseVirtualTableNum];
	if (pageTableEntry->reference == 0 && currentInst - frameTable[currentSelectedFrameIndex].lastUsedTime > 49) {
		// abort searching
		return &frameTable[currentSelectedFrameIndex];
	}
	else if(pageTableEntry->reference == 1){
		pageTableEntry->reference = 0;
		frameEntry->lastUsedTime = currentInst;
	}

	for (; hand != start; hand++) {
		if (hand == frameTableSize) {
			hand = 0;
			if (hand == start) {
				break;
			}
		}
		FrameTableEntry* frameEntry = &frameTable[hand];
		PageTabelEntry* pageTableEntry = &processes.at(frameEntry->reverseProcessNum)->pageTable[frameEntry->reverseVirtualTableNum];
		if (pageTableEntry->reference == 1) {
			pageTableEntry->reference = 0;
			frameEntry->lastUsedTime = currentInst;
			continue;
		}
		else {
			if (currentInst - frameEntry->lastUsedTime > 49) {
				// find a entry, abort searching
				currentSelectedFrameIndex = hand;
				hand = currentSelectedFrameIndex + 1 >= frameTableSize ? 0 : currentSelectedFrameIndex + 1;
				return &frameTable[currentSelectedFrameIndex];
			}
			else {
				if (frameEntry->lastUsedTime < frameTable[currentSelectedFrameIndex].lastUsedTime) {
					currentSelectedFrameIndex = hand;
				}
			}
		}
	}
	hand = currentSelectedFrameIndex + 1 >= frameTableSize ? 0 : currentSelectedFrameIndex + 1;
	return &frameTable[currentSelectedFrameIndex];
}