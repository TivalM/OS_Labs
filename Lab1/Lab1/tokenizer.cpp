#include "tokenizer.h"
#include <limits.h>
#include <iostream>
#include <regex>

tokenizer::tokenizer(char* filename) {
	row = offset = finalSpotLine = finalSpotOffset = 0;
	lineToProcess = NULL;
	token = NULL;
	infile.open(filename);
}

char* tokenizer::getToken() {
	if (lineToProcess == NULL) {
		if (infile.eof()) {
			// reached eof
			return NULL;
		}
		getline(infile, strLine);
		if (strLine.empty()) {
			if (infile.eof()) {
				// skip last empty line
				return NULL;
			}
		}
		lineToProcess = (char*)strLine.data();
		token = strtok(lineToProcess, " \t\n");
		row++;
		finalSpotLine = row;
		finalSpotOffset = offset = 1;
	}
	else {
		token = strtok(NULL, " \t\n");
	}
	if (token != NULL) {
		offset = token - lineToProcess + 1;
		finalSpotOffset = offset + strlen(token);
		return token;
	}
	else {
		//no token in current line
		lineToProcess = NULL;
		getToken();
	}
}

int tokenizer::readInt() {
	char* token = getToken();
	if (token != NULL) {
		//if every digit of c is num, then c is num
		bool flag = true;
		for (int i = 0; i < strlen(token); i++) {
			if (!isdigit(token[i])) {
				flag = false;
			}
		}
		return flag ? std::stoi(token) : NULL;
	}
	return NULL;
}

char* tokenizer::readSymbol() {
	char* token = getToken();
	if (token != NULL) {
		//if every digit of c is num, then c is num		
		if (std::regex_match(token, std::regex("^[a-z]([a-z]|[0-9])*", std::regex::icase))) {
			return token;
		}
	}
	return NULL;
}

char tokenizer::readIAER() {
	char* token = getToken();
	if (token != NULL && strlen(token) == 1) {
		//if every digit of c is num, then c is num
		if (token[0] == 'I' || token[0] == 'A' || token[0] == 'E' || token[0] == 'R') {
			return token[0];
		}
	}
	return NULL;
}

int tokenizer::getRow() {
	return row;
}

int tokenizer::getOffset() {
	return offset;
}

int tokenizer::getFinalSpotLine() {
	if (infile.eof()) {
		return finalSpotLine;
	}
	return -1;
}

int tokenizer::getFinalSpotOffset() {
	if (infile.eof()) {
		return finalSpotOffset;
	}
	return -1;
}
