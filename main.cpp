#include "headers.h"

int main(int argc, char *argv[]))
{
	if (argc == 0) {
		cout << "Invalid syntax. Use: " << endl
				<< "DangIt.exe \"<file name>\"";
		return 1;
	}
	ifstream file(argv[0], ifstream::in);
	
	if (!file.good()) {
		cout << "Can not open file \"" << argv[0] << "\".";
		return 1;
	}
	
	parseScript(file);
	file.close();
	return 0;
}