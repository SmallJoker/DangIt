#include "headers.h"

int parseScript(ifstream &file)
{
	
}

string readToNext(ifstream &file, READ_METHOD mod)
{
	bool start_read = false;
	bool text_start = false;
	char c;
	stringstream text;
	
	while (file.get(c)) {
		bool valid_char = false;
		bool is_space = c <= ' ';
		
		switch (mod) {
		case READ_METHOD::NAME:
			valid_char = (c >= 'A' && c <= 'Z')
					|| (c >= 'a' && c <= 'z')
					|| (start_read && c >= '0' && c <= '9');
		break;
		case READ_METHOD::VALUE:
			valid_char = (c >= 'A' && c <= 'Z')
					|| (c >= 'a' && c <= 'z')
					|| (start_read && c >= '0' && c <= '9')
					|| c == '"';
		break;
		case READ_METHOD::OPERATOR:
			valid_char = (c == '+' || c == '-')
					|| (c == '*' || c == '/');
		break;
		}
		if (!start_read && !is_space)
			start_read = true;
		
		if (start_read) {
			if (!valid_char) {
				file.putback(c);
				break;
			}
			
			if (c == '"') {
				if (!text_start) {
					text_start = true;
					continue;
				} else {
					break;
				}
			}
			
			text << c;
		} else if (!valid_char) {
			// Invalid character, throw error
			break;
		}
	}
	
	return text.str();
}