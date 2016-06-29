// cout
#include <iostream>

// ifstream
#include <fstream>

#include <string>
// stringstream
#include <sstream>

using namespace std;

int parseScript(ifstream &file);

enum READ_METHOD {
	NAME,
	VALUE
}