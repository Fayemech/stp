#include <stdio.h>      /* for printf() and fprintf() */
#include <stdlib.h>     /* for atoi() and exit() */
#include <string.h>     /* for memset() */
#include <errno.h>      /* for errno and EINTR */
#include <signal.h>     /* for sigaction() */
#include <iostream>
#include <vector>
#include <unordered_map>
#include <fstream>
#include <string>
#include <queue>
#include <sstream>
#include <iomanip>

using namespace std;

int main(int argc, char* argv[]) {

	/* check to see if the input parameter right*/
	string t = argv[1];
	string key = argv[2];
	/*cout << t << endl << key << endl;*/
	if (argc != 3)
	{
		cout << "error input!";
		return 0;
	}
	string name1, name2;
	cout << "type the test files name" << endl;
	cin >> name1;
	cout << "type the output files name" << endl;
	cin >> name2;
	/* read the input and creat the output file*/
	ifstream infile;
	ofstream out(name2);
	infile.open(name1);
	string s;

	/* read the key*/


		/* do it */
	if (t == "-e") {
		while (getline(infile, s)) {
			for (int i = 0; i < s.size(); i++) {
				if (s[i] >= 'a' && s[i] <= 'z') {
					int ind = key[i % key.size()] - '0';
					char b = (s[i] - 'a' + ind) % 26 + 'a';
					s[i] = b;
				}
				else if (s[i] >= 'A' && s[i] <= 'Z') {
					int ind = key[i % key.size()] - '0';
					char b = (s[i] - 'A' + ind) % 26 + 'A';
					s[i] = b;
				}
			}
			out << s << endl;
		}
	}
	else if (t == "-d") {
		while (getline(infile, s)) {
			for (int i = 0; i < s.size(); i++) {
				if (s[i] >= 'a' && s[i] <= 'z') {
					int ind = key[i % key.size()] - '0';
					char b = (s[i] - 'a' + (26 - ind)) % 26 + 'a';
					s[i] = b;
				}
				else if (s[i] >= 'A' && s[i] <= 'Z') {
					int ind = key[i % key.size()] - '0';
					char b = (s[i] - 'A' + (26 - ind)) % 26 + 'A';
					s[i] = b;
				}
			}
			out << s << endl;
		}
	}


	return 0;
}
