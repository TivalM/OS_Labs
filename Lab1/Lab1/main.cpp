#include <stdio.h>
#include <string.h>

#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include "tokenizer.h"

using namespace std;

void pass1();
void processSymbol(int& symblelIndex, int addressOffset, int modelCount, int modelLength);
void parseError(int errcode, Tokenizer& tokenizer);
void printSymbleTable();

struct TableEntry
{
public:
	string symble;
	int address;
	bool isDefinedMutiltTimes;
	bool operator==(const TableEntry& entry) const {
		return entry.symble == (symble);
	}
};

char* filename = (char*)"G:\\Project\\NYU\\OS\\lab1\\reference\\lab1samples\\input-21";
vector<TableEntry> symbleTable;

int main(int argc, char* argv[]) {
	//filename = argv[1];
	cout << "fileName: " << filename << endl;
	pass1();
	//Tokenizer tokenizer(filename);
	//while (!tokenizer.isEndOfFile()) {
	//	char* a = tokenizer.getToken();
	//	if (a != NULL) {
	//		cout << tokenizer.getRow() << ":" << tokenizer.getOffset() << " " << a << endl;
	//	}
	//	//		cout << tokenizer.getRow() << ":" << tokenizer.getOffset() << " " << token << endl;
	//	//		token = tokenizer.getToken();
	//}
	//cout << "Final Spot in File: "
	//	<< "line=" << tokenizer.getFinalSpotLine() << " offset=" << tokenizer.getFinalSpotOffset() << endl;
}

void pass1() {
	Tokenizer tokenizer(filename);
	int modelStartOffset = 0;
	int modelCount = 0;
	int symblelIndex = 0;
	int totalInstrNum = 0;
	while (!tokenizer.isEndOfFile()) {
		modelCount++;
		int defCount = tokenizer.readInt();
		if (defCount != INT_MIN) {
			if (defCount < 16 || defCount == 16) {
				for (int i = 0; i < defCount; i++) {
					char* symbol = tokenizer.readSymbol();
					if (symbol != NULL) {
						if (strlen(symbol) > 16) {
							parseError(3, tokenizer);
						}
						int val = tokenizer.readInt();
						if (val != INT_MIN) {
							TableEntry tmpEntry;
							tmpEntry.symble = *&symbol;
							tmpEntry.address = val;
							if (find(symbleTable.begin(), symbleTable.end(), tmpEntry) == symbleTable.end()) {
								tmpEntry.isDefinedMutiltTimes = false;
								symbleTable.push_back(tmpEntry);
							}
							else {
								find(symbleTable.begin(), symbleTable.end(), tmpEntry)->isDefinedMutiltTimes = true;
							}
						}
						else {
							parseError(0, tokenizer);
						}
					}
					else {
						parseError(1, tokenizer);
					}
				}
			}
			else {
				parseError(4, tokenizer);
			}
		}
		else {
			if (tokenizer.isEndOfFile()) {
				break;
			}
			else {
				parseError(0, tokenizer);
			}
		}
		int useCount = tokenizer.readInt();
		if (useCount != INT_MIN) {
			if (useCount < 16 || useCount == 16) {
				for (int i = 0; i < useCount; i++) {
					char* symbol = tokenizer.readSymbol();
					if (symbol != NULL) {
						if (strlen(symbol) > 16) {
							parseError(3, tokenizer);
						}
					}
					else {
						parseError(1, tokenizer);
					}
				}
			}
			else {
				parseError(5, tokenizer);
			}
		}
		else {
			parseError(0, tokenizer);
		}
		int instCount = tokenizer.readInt();
		if (instCount != INT_MIN) {
			totalInstrNum += instCount;
			if (totalInstrNum < 512) {
				for (int i = 0; i < instCount; i++) {
					char addressMode = tokenizer.readIAER();
					if (addressMode != NULL) {
						int oprand = tokenizer.readInt();
						if (oprand != INT_MIN) {

						}
						else {
							parseError(0, tokenizer);
						}
					}
					else {
						parseError(2, tokenizer);
					}
				}
			}
			else {
				parseError(6, tokenizer);
			}
			processSymbol(symblelIndex, modelStartOffset, modelCount, instCount);
			modelStartOffset += instCount;
		}
		else {
			parseError(0, tokenizer);
		}
	}
	printSymbleTable();
}

void processSymbol(int& symblelIndex, int addressOffset, int modelCount, int modelLength) {

	for (int i = symblelIndex; i < symbleTable.size(); i++) {
		if (symbleTable[i].address > modelLength - 1) {
			cout << "Warning: Module" << modelCount << ": " << symbleTable[i].symble << " too big " << symbleTable[i].address << " (max=" << modelLength - 1 << ") assume zero relative\n";
			symbleTable[i].address = addressOffset;
		}
		else {
			symbleTable[i].address += addressOffset;
		}
		symblelIndex++;
	}

}

void printSymbleTable() {
	cout << "Symbol Table" << endl;
	for (int i = 0; i < symbleTable.size(); i++) {
		cout << symbleTable[i].symble << "=" << symbleTable[i].address;
		if (symbleTable[i].isDefinedMutiltTimes) {
			cout << " Error: This variable is multiple times defined; first value used" << endl;
		}
		else {
			cout << endl;
		}
	}
}

void parseError(int errcode, Tokenizer& tokenizer) {
	static const char* errstr[] = {
		"NUM_EXPECTED",      		// Number expect
		"SYM_EXPECTED", 			// Symbol Expected
		"ADDR_EXPECTED",			// Addressing Expected which is A/E/I/R  
		"SYM_TOO_LONG",				// Symbol Name is too long
		"TOO_MANY_DEF_IN_MODULE",	// > 16 
		"TOO_MANY_USE_IN_MODULE",   // > 16
		"TOO_MANY_INSTR"			// total num_instr exceeds memory size (512)  
	};
	if (tokenizer.isEndOfFile()) {
		printf("Parse Error line %d offset %d: %s\n", tokenizer.getFinalSpotLine(), tokenizer.getFinalSpotOffset(), errstr[errcode]);
	}
	else {
		printf("Parse Error line %d offset %d: %s\n", tokenizer.getRow(), tokenizer.getOffset(), errstr[errcode]);
	}
	exit(0);
}
