#include "tokenizer.h"
#include <limits.h>
#include <iostream>
#include <regex>

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
}

int Tokenizer::readInt() {
	getToken();
	if (currentToken != NULL) {
		//if every digit of c is num, then c is num
		bool flag = true;
		for (int i = 0; i < strlen(currentToken); i++) {
			if (!isdigit(currentToken[i])) {
				flag = false;
			}
		}
		if (flag) {
			return std::stoi(currentToken);
		}

	}

	return INT_MIN;
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
