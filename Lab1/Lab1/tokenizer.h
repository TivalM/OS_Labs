#pragma once
#include <stdio.h>
#include <string.h>

#include <fstream>
#include <string>
static char IAER[]{'I','A','E','R'};

class tokenizer
{
public:
	tokenizer(char*);
	char* getToken();
	int readInt();
	char* readSymbol();
	char readIAER();
	int getRow();
	int getOffset();
	int getFinalSpotLine();
	int getFinalSpotOffset();

private:
	int row;
	int offset;
	int finalSpotLine;
	int finalSpotOffset;
	char* token;
	char* lineToProcess;
	std::ifstream infile;
	std::string strLine;
};

