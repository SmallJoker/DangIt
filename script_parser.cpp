// scriptParser.cpp - Parse a stream of characters and make something with it
// Most important place to put stuff in so far
#include "headers.h"
#include <algorithm>

#define PRINT_DEBUG(c, pd)              \
	   "\n\tChar: " << (c)              \
	<< "\n\tLine: " << (pd).line_nr     \
	<< "\n\tPos:  " << (pd).is->tellg() \
	<< endl

inline const OpCodeInformation* getOperator(ParseData &pd, char c)
{
	int8_t &balance = pd.balance.top();
	const OpCodeInformation *inf = nullptr;

	// Hacky
	std::string txt(to_string(c));
	char d = pd.is->peek();
	if (d == '=' || d == '-' || d == '+') {
		txt.append(to_string(d));
		pd.is->get(d);
	}

	int balance_diff = 0;
	for (const OpCodeInformation &i : op_lookup) {
		if (!i.pattern || i.pattern != txt)
			continue;

		int l_diff = balance - std::max(i.nary, (int8_t)0);
		if (l_diff < balance_diff) {
			inf = &i;
			balance_diff = l_diff;
		}
	}
	if (inf) {
		balance -= inf->nary;
		return inf;
	}

	switch (c) {
	case '(':
		pd.parentheses++;
		pd.balance.push(0);
		break;
	case ')':
		pd.parentheses--;
		if (pd.parentheses < 0)
			errorstream << "Unexpected character " << PRINT_DEBUG(c, pd);
		if (balance != 0)
			errorstream << "Operation balance != 0 " << PRINT_DEBUG(c, pd);
		pd.balance.pop();
		break;
	case ';':
		if (pd.parentheses > 0)
			errorstream << "Unclosed parentheses" << PRINT_DEBUG(c, pd);
		if (balance != 0)
			errorstream << "Operation balance != 0 " << PRINT_DEBUG(c, pd);
		break;
	default:
		// Unknown expression
		errorstream << "Unknown expression" << PRINT_DEBUG(c, pd);
		break;
	}

	return nullptr;
}

void readNextFromRaw(ParseData &pd, VariableList &vl, ElementList &el)
{
	stringstream value;
	const OpCodeInformation *operation = nullptr;
	uint8_t etype = 0;
	uint8_t comment_type = 0;
	bool string_escape = false;
	char c;

	while (pd.is->get(c)) {

	#if 1 // Skip spaces and comments
		if (isspace(c)) {
			if (c == '\n') {
				pd.line_nr++;

				// Reset single line comments
				if (comment_type == 1) {
					comment_type = 0;
					continue;
				}
			}
			// Wait for it (= the element)
			if (etype == ETYPE::NONE)
				continue;
			if (etype != ETYPE::STRING)
				break;
		}

		// Handle multiline comments
		if (comment_type == 2) {
			if (c == '*' && pd.is->peek() == '/') {
				pd.is->get(c); // Skip '/'

				// TODO: Check what ` /*/ ` does (ultrashort multiline comment)
				comment_type = 0;
			}
			continue;
		}
		if (c == '/' && etype != ETYPE::STRING) {
			char next = pd.is->peek();
			if (next == '/') {
				// Singleline comment
				comment_type = 1;
				continue;
			} else if (next == '*') {
				// Multiline comment
				comment_type = 2;
				continue;
			}
		}
	#endif

		switch (etype) {
		case ETYPE::NONE: {
			// Handle string detection first to not get '"' in our string value
			if (c == '"') {
				etype = ETYPE::STRING;
				break;
			}
			value << c;
			if (isalpha(c) || c == '_') {
				etype = ETYPE::VARIABLE;
				break;
			}
			if (c >= '0' && c <= '9') {
				// Can be changed later to float when a wild '.' appears
				etype = ETYPE::INT;
				break;
			}
			if (c == '.') {
				etype = ETYPE::FLOAT;
				break;
			}
			// Otherwise: Operator
			operation = getOperator(pd, c);
			if (!operation)
				return; // Nothing to add

			etype = ETYPE::OPERATION;
			// Reading complete
			goto read_complete;
			}
		break;
		case ETYPE::STRING:
			if (c == '"') {
				if (string_escape) {
					value << c;
					string_escape = false;
				} else {
					// String complete
					goto read_complete;
				}
				break;
			}
			if (c == '\\') {
				string_escape ^= true;
				if (string_escape)
					break; // Ignore escape character
			}
			// Extend string
			value << c;
		break;
		case ETYPE::INT:
		case ETYPE::FLOAT:
			if (c == '.') {
				if (etype == ETYPE::INT) {
					// Apparently it's floating point
					etype = ETYPE::FLOAT;
				} else { // type == ETYPE::FLOAT
					// There are no two decimal points in a decimal
					warningstream << "Invalid number near " << PRINT_DEBUG(c, pd);
				}
				value << c;
				break;
			}
			if (c >= '0' && c <= '9') {
				value << c;
				break;
			}
			warningstream << "Invalid number near " << c << PRINT_DEBUG(c, pd);
		break;
		case ETYPE::VARIABLE:
			if (isalnum(c) || c == '_') {
				value << c;
				break;
			}
			errorstream << "Invalid variable character: " << PRINT_DEBUG(c, pd);
		break;
		default:
			warningstream << "Type not implemented: " << etype << PRINT_DEBUG(c, pd);
			return;
		}
	}

read_complete:

	const string &text = value.str();
	Element e = {
		.type = etype,
		.flags = 0,
		.priority = (uint16_t)(pd.parentheses << 8),
		.val = {0}
	};
	if (operation)
		e.priority |= operation->opcode;

	switch (etype) {
	case ETYPE::NONE:
		warningstream << "Tried to add type NONE. STOP." << endl;
	return;
	case ETYPE::STRING:
		e.val.sp = new string(text);
	break;
	case ETYPE::INT:
		e.val.s = atoi(text.c_str());
	break;
	case ETYPE::FLOAT:
		e.val.f = atof(text.c_str());
	break;
	case ETYPE::VARIABLE:
		// TODO: 'if', 'else', 'goto' and such

		// Try to find the variable
		for (VariableList::const_iterator it = vl.begin();
				it != vl.end(); ++it) {
			if (it->name == text) {
				verbosestream << "Found object '" << text << "'" << endl;
				e.val.it = it;
				break;
			}
		}

		// Object does not exist
		errorstream << "Object '" << text << "' was not found" << endl;
	break;
	}

	ElementList::iterator it = el.end();
	if (el.size() > 0 && pd.balance.top() < 0) {
		Element back = el.back();
		if (back.type == ETYPE::OPERATION) {
			/*uint8_t opcode = back.priority & 0xFF;
			for (const OpCodeInformation &inf : op_lookup) {
				if (inf.opcode != opcode)
					continue;

				// TODO: only works for nary = -1
				if (inf.nary < 0)
					it--;
				break;
			}*/
			it--;
		} else {
			errorstream << "Balance destroyed. What do do?" << endl;
		}
	}
	el.insert(it, e);

	if (etype != ETYPE::OPERATION)
		pd.balance.top()++; // +1 for values, -x for operators
}
