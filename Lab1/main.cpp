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
#include "tokenizer.h"

struct TableEntry
{
public:
	std::string symble;
	int address = -1;
	int moduelBelogsTo;
	bool isDefinedMutiltTimes = false;
	bool isUsed = false;		//use in pass 2 
	bool addInPass2 = false;	//use in pass 2 
	bool operator==(const TableEntry& entry) const {
		return entry.symble == (symble);
	}
};

void pass1();
void pass2();
void processSymbol(int& symblelIndex, std::vector<TableEntry>& subList, int addressOffset, int modelCount, int modelLength);
void processInstruction(std::vector<TableEntry>& useList, int memoryIndex, int startOffset, int, int oprand, char addressMode);
void parseError(int errcode, Tokenizer& tokenizer);
void printSymbleTable();

char* filename;
std::vector<TableEntry> symbleTable;

int main(int argc, char* argv[]) {
	filename = argv[1];
	pass1();
	pass2();
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
		std::vector<TableEntry> subList;
		int defCount = tokenizer.readInt();
		if (defCount != INT_MIN) {
			modelCount++;
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
							tmpEntry.moduelBelogsTo = modelCount;
							subList.push_back(tmpEntry);
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
			processSymbol(symblelIndex, subList, modelStartOffset, modelCount, instCount);
			modelStartOffset += instCount;
		}
		else {
			parseError(0, tokenizer);
		}
	}
	printSymbleTable();
}

void pass2() {
	Tokenizer tokenizer(filename);
	int modelStartOffset = 0;
	int modelCount = 0;
	int instrCount = 0;
	int totalInstrNum = 0;
	std::cout << "Memory Map" << std::endl;
	while (!tokenizer.isEndOfFile()) {
		std::vector<TableEntry> useList;
		int defCount = tokenizer.readInt();
		if (defCount != INT_MIN) {
			modelCount++;
			if (defCount < 16 || defCount == 16) {
				for (int i = 0; i < defCount; i++) {
					char* symbol = tokenizer.readSymbol();
					if (symbol != NULL) {
						if (strlen(symbol) > 16) {
							parseError(3, tokenizer);
						}
						int val = tokenizer.readInt();
						if (val != INT_MIN) {
							//todo
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
						else {
							TableEntry tmpEntry;
							tmpEntry.symble = symbol;
							if (find(symbleTable.begin(), symbleTable.end(), tmpEntry) != symbleTable.end()) {
								//already exist in symble table
							}
							else {
								tmpEntry.address = 0; //use zero as address
								tmpEntry.addInPass2 = true;
								symbleTable.push_back(tmpEntry);
							}
							useList.push_back(tmpEntry);
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
						bool appendValueOverflowError = false;
						int oprand = tokenizer.readInt();
						if (oprand != INT_MIN) {
							processInstruction(useList, i, totalInstrNum - instCount, instCount, oprand, addressMode);
						}
						else {
							parseError(0, tokenizer);
						}
					}
					else {
						parseError(2, tokenizer);
					}
				}
				for (int i = 0; i < useList.size(); i++) {
					if (!useList[i].isUsed) {
						std::cout << "Warning: Module " << modelCount << ": " << useList[i].symble << " appeared in the uselist but was not actually used" << std::endl;
					}
				}
			}
			else {
				parseError(6, tokenizer);
			}
			modelStartOffset += instCount;
		}
		else {
			parseError(0, tokenizer);
		}

	}
	std::cout << std::endl;
	for (int i = 0; i < symbleTable.size(); i++) {
		if (!symbleTable[i].isUsed && !symbleTable[i].addInPass2) {
			std::cout << "Warning: Module " << symbleTable[i].moduelBelogsTo << ": " << symbleTable[i].symble << " was defined but never used" << std::endl;
		}
	}
}

void processInstruction(std::vector<TableEntry>& useList, int modelInstrIndex, int startOffset, int modelSize, int oprand, char addressMode) {
	if (addressMode == 'I') {
		bool isValueOverflow = false;
		if (oprand > 9999) {
			oprand = 9999;
			isValueOverflow = true;
		}
		std::cout << std::setfill('0') << std::setw(3) << startOffset + modelInstrIndex << ": " << std::setw(4) << oprand;
		if (isValueOverflow) {
			std::cout << " Error: Illegal immediate value; treated as 9999";
		}
		std::cout << std::endl;
	}
	else if (addressMode == 'A') {
		bool isValueOverflow = false;
		bool isOpOverflow = false;

		if (oprand / 1000 > 9) {
			oprand = 9999;
			isOpOverflow = true;
		}
		if (!isOpOverflow && oprand % 1000 > 511) {
			oprand = oprand / 1000 * 1000;
			isValueOverflow = true;
		}
		std::cout << std::setfill('0') << std::setw(3) << startOffset + modelInstrIndex << ": " << std::setw(4) << oprand;
		if (isOpOverflow) {
			std::cout << " Illegal opcode; treated as 9999";
		}
		else if (isValueOverflow) {
			std::cout << " Error: Absolute address exceeds machine size; zero used";
		}
		std::cout << std::endl;
	}
	else if (addressMode == 'E') {
		if (oprand / 1000 > 9) {
			std::cout << std::setfill('0') << std::setw(3) << startOffset + modelInstrIndex << ": " << "9999";
			std::cout << " Error: Illegal opcode; treated as 9999";
		}
		else if (oprand % 1000 > useList.size() - 1) {
			std::cout << std::setfill('0') << std::setw(3) << startOffset + modelInstrIndex << ": " << std::setw(4) << oprand;
			std::cout << " Error: External address exceeds length of uselist; treated as immediate";
		}
		else {
			int entryIndex = oprand % 1000;
			useList[entryIndex].isUsed = true;
			find(symbleTable.begin(), symbleTable.end(), useList[entryIndex])->isUsed = true;

			int address = find(symbleTable.begin(), symbleTable.end(), useList[entryIndex])->address;
			bool isAddedInPass2 = find(symbleTable.begin(), symbleTable.end(), useList[entryIndex])->addInPass2;
			oprand = oprand / 1000 * 1000 + address;
			std::cout << std::setfill('0') << std::setw(3) << startOffset + modelInstrIndex << ": " << std::setw(4) << oprand;
			if (isAddedInPass2) {
				std::string symble = find(symbleTable.begin(), symbleTable.end(), useList[entryIndex])->symble;
				std::cout << " Error: " << symble << " is not defined; zero used";
			}
		}
		std::cout << std::endl;
	}
	else if (addressMode == 'R') {
		if (oprand / 1000 > 9) {
			std::cout << std::setfill('0') << std::setw(3) << startOffset + modelInstrIndex << ": " << "9999";
			std::cout << " Error: Illegal opcode; treated as 9999";
		}
		else if (oprand % 1000 > modelSize - 1) {
			oprand = oprand / 1000 * 1000 + startOffset;
			std::cout << std::setfill('0') << std::setw(3) << startOffset + modelInstrIndex << ": " << std::setw(4) << oprand;
			std::cout << " Error: Relative address exceeds module size; zero used";
		}
		else {
			oprand = oprand / 1000 * 1000 + startOffset + oprand % 1000;
			std::cout << std::setfill('0') << std::setw(3) << startOffset + modelInstrIndex << ": " << std::setw(4) << oprand;
		}
		std::cout << std::endl;
	}
}

void processSymbol(int& symblelIndex, std::vector<TableEntry>& subList, int addressOffset, int modelCount, int modelLength) {
	for (int i = 0; i < subList.size(); i++) {
		if (find(symbleTable.begin(), symbleTable.end(), subList[i]) != symbleTable.end()) {
			find(symbleTable.begin(), symbleTable.end(), subList[i])->isDefinedMutiltTimes = true;
			subList[i].address = find(symbleTable.begin(), symbleTable.end(), subList[i])->address-addressOffset;
		}
		else {
			subList[i].isDefinedMutiltTimes = false;
			symbleTable.push_back(subList[i]);
		}
		if (subList[i].address > modelLength - 1) {
			std::cout << "Warning: Module " << modelCount << ": " << subList[i].symble << " too big " << subList[i].address << " (max=" << modelLength - 1 << ") assume zero relative" << std::endl;
		}
	}
	for (symblelIndex; symblelIndex < symbleTable.size(); symblelIndex++) {
		if (symbleTable[symblelIndex].address > modelLength - 1) {
			symbleTable[symblelIndex].address = addressOffset;
		}
		else {
			symbleTable[symblelIndex].address += addressOffset;
		}
	}
}

void printSymbleTable() {
	std::cout << "Symbol Table" << std::endl;
	for (int i = 0; i < symbleTable.size(); i++) {
		std::cout << symbleTable[i].symble << "=" << symbleTable[i].address;
		if (symbleTable[i].isDefinedMutiltTimes) {
			std::cout << " Error: This variable is multiple times defined; first value used" << std::endl;
		}
		else {
			std::cout << std::endl;
		}
	}
	std::cout << std::endl;
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
