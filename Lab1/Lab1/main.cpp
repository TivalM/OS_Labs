#include <stdio.h>
#include <string.h>

#include <fstream>
#include <iostream>
#include <string>
#include "tokenizer.h"

using namespace std;

char* filename;

int main(int argc, char* argv[]) {
	filename = argv[1];
	cout << "fileName: " << filename << endl;
	tokenizer tokenizer(filename);
//	char* token = tokenizer.getToken();
	while (true) {
		char* a = tokenizer.readSymbol();
		if (a != NULL) {
			cout << tokenizer.getRow() << ":" << tokenizer.getOffset() << a <<  endl;
		}
//		cout << tokenizer.getRow() << ":" << tokenizer.getOffset() << " " << token << endl;
//		token = tokenizer.getToken();
	}
	cout << "Final Spot in File: "
		<< "line=" << tokenizer.getFinalSpotLine() << " offset=" << tokenizer.getFinalSpotOffset() << endl;
}