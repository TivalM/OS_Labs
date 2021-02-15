#include <stdio.h>
#include <string.h>

#include <fstream>
#include <iostream>
#include <string>
using namespace std;

char* fileName;
void tokenizer(char* fileName);

int main(int argc, char* argv[]) {
	fileName = argv[1];
	cout << "fileName: " << fileName << endl;
	tokenizer(fileName);
}

void tokenizer(char* fileName) {
	int row = 1;
	int offset = 0;
	int finalSpotLine = 0;
	int finalSpotOffset = 0;

	char* token;
	ifstream infile;
	infile.open(fileName);
	string strLine;
	for (row = 1; getline(infile, strLine); row++) {
		if (strLine.empty()) {
			if (infile.eof()) {
				// skip last empty line
				continue;
			}
			finalSpotOffset = offset = 1;
		}
		offset = 1;
		char* lineToProcess = (char*)strLine.data();
		char* token = strtok(lineToProcess, " \t\n");
		while (token != NULL) {
			offset = token - lineToProcess + 1;
			cout << row << ":" << offset << " " << token << endl;
			finalSpotOffset = offset + strlen(token);
			token = strtok(NULL, " \t\n");
		}
		finalSpotLine = row;
	}
	cout << "Final Spot in File: "
		<< "line=" << finalSpotLine << " offset=" << finalSpotOffset << endl;
}