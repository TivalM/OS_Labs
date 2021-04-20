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

void Process::printProcessSummary(){
	printf("PROC[%d]: U=%lu M=%lu I=%lu O=%lu FI=%lu FO=%lu Z=%lu SV=%lu SP=%lu\n",
		pid, unmaps, maps, ins, outs, fins, 
		fouts, zeros, segv, segprot);
}

FrameTableEntry* FIFO::selectVictimFrame(unsigned long currentInst, deque<Process*>& processes, FrameTableEntry* frameTable, int frameTableSize, int randomNum)
{
	//hand [0, frameTableSize)
	if (hand == frameTableSize) {
		hand = 0;
	}
	return &frameTable[hand++];
}

FrameTableEntry* CLOCK::selectVictimFrame(unsigned long currentInst, deque<Process*>& processes, FrameTableEntry* frameTable, int frameTableSize, int randomNum)
{
	//hand [0, frameTableSize)
	if (hand == frameTableSize) {
		hand = 0;
	}
	FrameTableEntry* entry = &frameTable[hand++];
	PageTabelEntry* pageTableEntry = &processes.at(entry->reverseProcessNum)->pageTable[entry->reverseVirtualTableNum];
	if (pageTableEntry->reference == 1){
		pageTableEntry->reference = 0;
		selectVictimFrame(currentInst, processes, frameTable, frameTableSize, randomNum);
	}
	else{
		return entry;
	}
}

FrameTableEntry* NRU::selectVictimFrame(unsigned long currentInst, deque<Process*>& processes, FrameTableEntry* frameTable, int frameTableSize, int randomNum)
{
	bool needResetReferenceBit = (currentInst - lastCalledInst >= 50);
	if (needResetReferenceBit){
		lastCalledInst = currentInst;
	}

	FrameTableEntry* frameEntry = &frameTable[hand];
	PageTabelEntry* pageTableEntry = &processes.at(frameEntry->reverseProcessNum)->pageTable[frameEntry->reverseVirtualTableNum];
	int start = hand;

	//try to find a better one than the beginning
	unsigned int currentSelectedFrameIndex = hand++;
	unsigned int selectedFrameModified = pageTableEntry->modified;
	unsigned int selectedFrameReferenced = pageTableEntry->reference;

	if (pageTableEntry->modified == 0 && pageTableEntry->reference == 0 && !needResetReferenceBit) {
		hand = currentSelectedFrameIndex + 1 >= frameTableSize ? 0 : currentSelectedFrameIndex + 1;
		return frameEntry;
	}
	if (needResetReferenceBit){
		pageTableEntry->reference = 0;
	}

	for (;hand != start; hand++){
		if (hand == frameTableSize) {
			hand = 0;
			if (hand == start){
				break;
			}
		}
		frameEntry = &frameTable[hand];
		pageTableEntry = &processes.at(frameEntry->reverseProcessNum)->pageTable[frameEntry->reverseVirtualTableNum];
		if (pageTableEntry->modified == 0 && pageTableEntry->reference == 0) {
			if (!needResetReferenceBit){
				return frameEntry;
			}
			else{
				currentSelectedFrameIndex = hand;
				selectedFrameModified = pageTableEntry->modified;
				selectedFrameReferenced = pageTableEntry->reference;
			}
		}
		else {
			if (pageTableEntry->reference < selectedFrameReferenced) {
				//select a better frame
				currentSelectedFrameIndex = hand;
				selectedFrameModified = pageTableEntry->modified;
				selectedFrameReferenced = pageTableEntry->reference;
			}
			else if(pageTableEntry->reference == selectedFrameReferenced){
				if (pageTableEntry->modified < selectedFrameModified){
					//select a better frame
					currentSelectedFrameIndex = hand;
					selectedFrameModified = pageTableEntry->modified;
					selectedFrameReferenced = pageTableEntry->reference;
				}
			}
			if (needResetReferenceBit && pageTableEntry->reference != 0){
				pageTableEntry->reference = 0;
			}
		}
	}
	hand = currentSelectedFrameIndex + 1 >= frameTableSize ? 0 : currentSelectedFrameIndex + 1;
	return &frameTable[currentSelectedFrameIndex];
}

FrameTableEntry* RANDOM::selectVictimFrame(unsigned long currentInst, deque<Process*>& processes, FrameTableEntry* frameTable, int frameTableSize, int randomNumber)
{
	return &frameTable[randomNumber];
}
