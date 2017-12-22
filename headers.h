#pragma once

#include <cctype>
#include <cstdlib> // atoi etc
#include <cstdint>
#include <string>

#include <iostream> // cout
#include <fstream> // ifstream
#include <sstream> // stringstream

#include <vector>
#include <stack>

// --- INCLUDES END ---


#define verbosestream cout << "VERBOSE: "
#define warningstream cout << "WARNING: "
#define errorstream   cout << "ERROR: "

using namespace std;

struct Variable; // Container for variables
struct Element;  // Data for operations

typedef vector<Variable> VariableList;
typedef vector<Element>  ElementList;

// --- DEFINITIONS END ---

struct OpCodeInformation {
	uint8_t opcode; // Higher = higher priority
	/* Amount of non-operator input elements,
	   has one output element */
	int8_t nary;
	const char *pattern;
};

extern OpCodeInformation op_lookup[25];


// [?OPCODE?][?OPCODE?][FLAGS][TYPE]
enum ETYPE : uint8_t {
	NONE = 0,
	OPERATION,
	INT,
	FLOAT,
	STRING,
	FUNCTION,
	VARIABLE,
	ARRAY
};

/* --- NOT ADDED YET ---
namespace VAR_FLAG
{
	def PUBLIC    = 0x0100;
	def CONSTANT  = 0x0200;
	def REFERENCE = 0x0400;
}*/

// Value of variable or static element
union Value {
	float    f;
	int32_t  s;
	// Variable reference
	VariableList::const_iterator it;
	// Priority for operator
	uint32_t u;
	string *sp;
	// For arrays?
	ElementList *vl;
};

// Operators and variable references
struct Element {
	uint8_t type;
	uint8_t flags;
	uint16_t priority;
	Value val;
};

// Variable values
struct Variable {
	uint32_t type;
	string name;
	Value val;
};

struct ParseData {
	ParseData(ifstream *is_) : is(is_)
	{
		balance.push(0);
	}

	ifstream *is;
	uint16_t line_nr = 0;
	uint16_t parentheses = 0;
	stack<int8_t> balance;
};

// --- END DATA TYPES ---


void simplifyElements(VariableList &vl, ElementList &el);
void readNextFromRaw(ParseData &pd, VariableList &vl, ElementList &el);
