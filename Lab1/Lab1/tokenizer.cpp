#include "tokenizer.h"
#include <climits>
#include <iostream>
#include <regex>
#define MAX_NUM_VALUE (1<<30)

Tokenizer::Tokenizer(char* filename) {
	row = offset = finalSpotLine = finalSpotOffset = 0;
	lineToProcess = NULL;
	currentToken = NULL;
	this->filename = filename;
	infile.open(filename);
}

char* Tokenizer::getToken() {
	if (lineToProcess == NULL) {
		if (infile.eof()) {
			// reached eof
			return NULL;
		}
		getline(infile, strLine);
		if (strLine.empty()) {
			if (infile.eof()) {
				// skip last empty line
				finalSpotOffset++;
				return NULL;
			}
		}
		lineToProcess = (char*)strLine.data();
		currentToken = strtok(lineToProcess, " \t\n");
		row++;
		finalSpotLine = row;
		finalSpotOffset = offset = 0;
	}
	else {
		currentToken = strtok(NULL, " \t\n");
	}
	if (currentToken != NULL) {
		offset = currentToken - lineToProcess + 1;
		finalSpotOffset = offset + strlen(currentToken) - 1;
		return currentToken;
	}
	else {
		//no token in current line
		lineToProcess = NULL;
		getToken();
	}
	return NULL;
}

int Tokenizer::readInt() {
	getToken();
	int sum = 0;
	if (currentToken != NULL) {
		//if every digit of c is num, then c is num
		for (int i = 0; i < strlen(currentToken); i++) {
			if (isdigit(currentToken[i])) {
				if (sum * 10 + (currentToken[i] - '0') >= MAX_NUM_VALUE) {
					//overflow
					return INT_MIN;
				}
				else {
					sum = sum * 10 + (currentToken[i] - '0');
				}
			}
			else {
				return INT_MIN;
			}
		}
	}
	else {
		return INT_MIN;
	}
	return sum;
}

char* Tokenizer::getCurrentToken() {
	return currentToken;
}

char* Tokenizer::readSymbol() {
	getToken();
	if (currentToken != NULL) {
		if (std::regex_match(currentToken, std::regex("^[a-z]([a-z]|[0-9])*", std::regex::icase))) {
			return currentToken;
		}

	}
	return NULL;
}

char Tokenizer::readIAER() {
	getToken();
	if (currentToken != NULL) {
		if (strlen(currentToken) == 1 && currentToken[0] == 'I' || currentToken[0] == 'A' || currentToken[0] == 'E' || currentToken[0] == 'R') {
			return currentToken[0];
		}
	}
	return NULL;
}

int Tokenizer::getRow() {
	return row;
}

int Tokenizer::getOffset() {
	return offset;
}

int Tokenizer::getFinalSpotLine() {
	return finalSpotLine;
}

int Tokenizer::getFinalSpotOffset() {
	return finalSpotOffset;
}

bool Tokenizer::isEndOfFile() {
	return (lineToProcess == NULL || strLine.empty()) && infile.eof();
}

void Tokenizer::resetState() {
	row = offset = finalSpotLine = finalSpotOffset = 0;
	lineToProcess = NULL;
	currentToken = NULL;
	infile.close();
	infile.open(filename);
}
