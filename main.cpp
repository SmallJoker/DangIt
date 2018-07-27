// main.cpp - Only for console-specific code
#include "headers.h"

void dump(const std::string &name, const ElementList &el)
{
	std::cout << "Dump of variable " << name << std::endl;
	for (const Element &e : el) {
		printf("\ttype=%i flags=%X prior=%i\n", e.type, e.flags, e.priority);
	}
}

#define DEBUG
int main(int argc, char *argv[])
{
#ifdef DEBUG
	ifstream is("hello.in", ifstream::in);
#else
	if (argc != 2) {
		errorstream << "Invalid syntax. Use: " << endl
				<< "\tDangIt \"<file name>\"" << endl;
		return 1;
	}
	ifstream is(argv[1], ifstream::in);
#endif

	if (!is.good()) {
		errorstream << "Can not open file \"" << argv[1] << "\"." << endl;
		return 1;
	}

	VariableList vars;
	ElementList elements;
	ElementList operator_stack;
	ParseData pd(&is);
	while (is.good())
		readNextFromRaw(pd, vars, elements, operator_stack);
	cout << "Balance: " << (int)pd.balance.top() << endl;
	dump("Elements", elements);
	dump("Operator Stack", operator_stack);

	//simplifyElements(vars, elements);
	/*
		number myvar = 33 * (5 - 2) + othervar

		1) Parse numbers, fill "variable bucket"
		2) Ensure clear calculation priorities (higher priority -> ++, else --)
		3) Calculate the stuff, from highest priority to zero
		4) Trash all non-variables (VARIABLE) -> name = ""
	*/
	is.close();
	return 0;
}
