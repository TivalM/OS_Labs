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

deque<Process*> processList;
int randCount = 0;
const char* inputFile = NULL;
const char* randFile = NULL;
std::ifstream inInputFile;
std::ifstream inRandFile;

int main(int argc, char** argv) {
	inputFile = "G:\\NYU\\OS\\Labs\\Lab3\\lab3_assign\\inputs\\in11";
	randFile = "G:\\NYU\\OS\\Labs\\Lab3\\lab3_assign\\inputs\\rfile";
	inInputFile.open(inputFile);
	inRandFile.open(randFile);
	inRandFile >> randCount;
	readAllProcess();

	for (int i = 0; i < processList.size(); i++)
	{
		processList.at(i)->printProcess();
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
	string strLine = "";
	//skip first few # lines and get process count
	while ((strLine == "" && !inInputFile.eof()) || strLine.rfind("#", 0) == 0) {
		getline(inInputFile, strLine);
	}
	int processNum = stoi(strLine);
	strLine = "";
	int a, b, c, d;
	for (int i = 0; i < processNum; i++){
		Process* process = new Process();
		while ((strLine == "" && !inInputFile.eof()) || strLine.rfind("#", 0) == 0) {
			getline(inInputFile, strLine);
		}
		int areaNum = stoi(strLine);
		strLine = "";
		for (int i = 0; i < areaNum; i++){
			inInputFile >> a >> b >> c >> d;
			process->addOneVirtualMemoryArea(a, b, c, d);
		}
		processList.push_back(process);
	}
}