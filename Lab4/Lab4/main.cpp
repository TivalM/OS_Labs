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
void readAllRequests();
void readNextInstrction();

int CURRENT_TIME;
int TOTAL_TIME = 0;
int TOTAL_MOVEMENT = 0;
double AVG_TURNAROUND = 0;
double AVG_WAITTIME = 0;
int MAX_WAITTIME = 0;
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
std::ifstream inInputFile;
string strLine;
int currentIOIssueTime;
int currentIOTrack;
bool readable = true;
deque<ioRequestEntry*>* ioRequestList = new deque<ioRequestEntry*>();
deque<ioRequestEntry*>* activeRequestList = new deque<ioRequestEntry*>();
deque<ioRequestEntry*>*   = new deque<ioRequestEntry*>();

Scheduler* scheduler;

bool FlagV = false;
bool FlagQ = false;
bool FlagF = false;

int main(int argc, char** argv) {
	//int c;
	//while ((c = getopt(argc, argv, "s:vqf")) != -1)
	//{
	//	switch (c) {
	//	case 'a':
	//		type = optarg[0];
	//		break;
	//	case 'v':
	//		FlagV = true;
	//		break;
	//	case 'q':
	//		FlagQ = true;
	//		break;
	//	case 'f':
	//		FlagF = true;
	//		break;
	//	}
	//}

	type = 'i';
	inputFile = "G:\\NYU\\OS\\Labs\\Lab4\\lab4_assign\\input0";
	//inputFile = argv[optind];
	inInputFile.open(inputFile);

	if (type == 'i') {
		scheduler = new FIFO();
	}
	else if (type == 'j') {
		scheduler = new SSTF();
	}
	else if (type == 's') {
		scheduler = new LOOK();
	}
	else if (type == 'c') {
		scheduler = new CLOOK();
	}
	else if (type == 'f') {
		scheduler = new FLOOK();
	}

	readAllRequests();
	//for (int i = 0; i < ioRequestList->size(); i++) {
	//	cout << ioRequestList->at(i)->issueTime << " " << ioRequestList->at(i)->track<<endl;
	//}
	unsigned int TRACK_POINTER = 0;
	unsigned int POINTER_START_FROM = 0;
	unsigned int WAIT_TIME = 0;
	int TOTAL_REQUEST_NUM = ioRequestList->size();
	ioRequestEntry* CURRENT_IO = nullptr;
	for (unsigned long timeStemp = 0; ; timeStemp++) {
		if (ioRequestList->size() != 0 && timeStemp == ioRequestList->at(0)->issueTime) {
			//a new I/O arrived
			activeRequestList->push_back(ioRequestList->at(0));
			ioRequestList->pop_front();
		}
		if (CURRENT_IO != nullptr) {
			if (TRACK_POINTER == CURRENT_IO->track) {
				//an IO is active and completed at this time
				CURRENT_IO->endTime = timeStemp;
				if (ioRequestList->size() == 0) {
					TOTAL_TIME = CURRENT_IO->endTime;
				}
				TOTAL_MOVEMENT += (labs(TRACK_POINTER - POINTER_START_FROM));
				POINTER_START_FROM = TRACK_POINTER;
				CURRENT_IO = nullptr;
			}
			else {
				//an IO is active but did not yet complete
				TRACK_POINTER = TRACK_POINTER < CURRENT_IO->track ? TRACK_POINTER + 1 : TRACK_POINTER - 1;
			}
		}
		if (CURRENT_IO == nullptr && activeRequestList->size() != 0) {
			//If no IO request active now (after (2)) but IO requests are pending
			CURRENT_IO = scheduler->getNextIO(ioRequestList, activeRequestList);
			WAIT_TIME += timeStemp - CURRENT_IO->issueTime;
			MAX_WAITTIME = timeStemp - CURRENT_IO->issueTime > MAX_WAITTIME ? timeStemp - CURRENT_IO->issueTime : MAX_WAITTIME;
		}
		if (ioRequestList->size() == 0 && CURRENT_IO == nullptr) {
			break;
		}
	}
	//print request summary
	for (int i = 0; i < ioRequestList->size(); i++) {
		printf("%5d: %5d %5d %5d\n", i, ioRequestList->at(i)->issueTime, ioRequestList->at(i)->startTime, ioRequestList->at(i)->endTime);
	}
	AVG_TURNAROUND = TOTAL_TIME / TOTAL_REQUEST_NUM;
	AVG_WAITTIME = WAIT_TIME / TOTAL_REQUEST_NUM;
	//print total summary
	printf("SUM: %d %d %.2lf %.2lf %d\n",
		TOTAL_TIME, TOTAL_MOVEMENT, AVG_TURNAROUND, AVG_WAITTIME, MAX_WAITTIME);
}

void readAllRequests() {
	//skip first few # lines
	strLine = "";
	while ((strLine == "" || strLine.rfind("#", 0) == 0) && !inInputFile.eof()) {
		getline(inInputFile, strLine);
	}
	if (strLine != "") {
		string delimiter = " ";
		string q = strLine.substr(0, strLine.find(delimiter));
		currentIOIssueTime = stoi(q);
		if (q.length() != strLine.length()) {
			strLine.erase(0, strLine.find(delimiter) + delimiter.length());
			currentIOTrack = stoi(strLine);
		}
		ioRequestEntry* entry = new ioRequestEntry(currentIOIssueTime, currentIOTrack);
		ioRequestList->push_back(entry);
	}
	strLine = "";
	while (true) {
		currentIOIssueTime = -1;
		if (!inInputFile.eof()) {
			inInputFile >> currentIOIssueTime >> currentIOTrack;
			if (currentIOIssueTime == -1) {
				break;
			}
			ioRequestEntry* entry = new ioRequestEntry(currentIOIssueTime, currentIOTrack);
			ioRequestList->push_back(entry);
		}
	}
}