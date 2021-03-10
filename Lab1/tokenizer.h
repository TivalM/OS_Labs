#pragma once
#include <stdio.h>
#include <string.h>

#include <fstream>
#include <string>
static char IAER[]{ 'I','A','E','R' };

class Tokenizer
{
public:
	Tokenizer(char*);
	int readInt();
	char* getCurrentToken();
	char* readSymbol();
	char readIAER();
	int getRow();
	int getOffset();
	int getFinalSpotLine();
	int getFinalSpotOffset();
	bool isEndOfFile();
	void resetState();
	char* getToken();

private:

	char* filename;
	int row;
	int offset;
	int finalSpotLine;
	int finalSpotOffset;
	char* currentToken;
	char* lineToProcess;
	std::ifstream infile;
	std::string strLine;
};

