// scriptParser.cpp - Parse a stream of characters and make something with it
// Most important place to put stuff in so far
#include "headers.h"
#include <algorithm>

#define PRINT_DEBUG(c, pd)              \
	   "\n\tChar: " << (c)              \
	<< "\n\tWhere: " << (pd).line_nr     \
	<< ":" << (pd).char_pos        \
	<< "\n\tBalance: " << (int)(pd).balance.top() \
	<< endl

inline bool getOperator(ParseData &pd, char c, const OpCodeInformation *&inf)
{
	const int8_t balance = pd.balance.top(); // Value balance for the current scope
	inf = nullptr;

	// Hacky
	std::string txt(1, c);
	char d = pd.is->peek();
	if (d == '=' || d == '-' || d == '+') {
		txt.append(1, d);
		pd.is->get(d);
	}

	int balance_diff = 1;
	for (const OpCodeInformation &i : op_lookup) {
		if (!i.pattern || i.pattern != txt)
			continue;

		int l_diff = balance - std::max(i.nary, (int8_t)0);
		// Find lowest balance change (prefer unary operations)
		if (l_diff < balance_diff) {
			inf = &i;
			balance_diff = l_diff;
		}
	}

	return inf ? true : false; // Read further
}

void readNextFromRaw(ParseData &pd, VariableList &vl, ElementList &el,
	ElementList &operator_stack)
{
	stringstream value;
	const OpCodeInformation *operation = nullptr;
	uint8_t etype = 0;
	uint8_t comment_type = 0;
	bool string_escape = false;
	char c;

	while (pd.is->get(c)) {
		pd.char_pos++;

	#if 1 // Skip spaces and comments
		if (isspace(c)) {
			if (c == '\n') {
				pd.line_nr++;
				pd.char_pos = 0;

				// Reset single line comments
				if (comment_type == 1) {
					comment_type = 0;
					continue;
				}
			}
			// Wait for it (= the element)
			if (etype == ETYPE::NONE)
				continue;
			//if (etype != ETYPE::STRING)
			//	break;
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
		case ETYPE::NONE:
			// Handle string detection first to not get '"' in our string value
			if (c == '"') {
				etype = ETYPE::STRING;
				continue;
			}
			value << c;
			if (isalpha(c) || c == '_') {
				etype = ETYPE::VARIABLE;
				continue;
			}
			if (c >= '0' && c <= '9') {
				// Can be changed later to float when a wild '.' appears
				etype = ETYPE::INT;
				continue;
			}
			if (c == '.') {
				etype = ETYPE::FLOAT;
				continue;
			}

			getOperator(pd, c, operation);

			// Otherwise: Operator
			if (operation) {
				etype = ETYPE::OPERATION;
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
				continue;
			}
			if (c == '\\') {
				string_escape ^= true;
				if (string_escape)
					continue; // Ignore escape character
			}
			// Extend string
			value << c;
		continue;
		case ETYPE::INT:
		case ETYPE::FLOAT:
			if (c == '.') {
				if (etype == ETYPE::INT) {
					// Apparently it's floating point
					etype = ETYPE::FLOAT;
				} else { // type == ETYPE::FLOAT
					// There are no two decimal points in a decimal
					errorstream << "Invalid number near " << PRINT_DEBUG(c, pd);
					return;
				}
				value << c;
				continue;
			}
			if (c >= '0' && c <= '9') {
				value << c;
				continue;
			}
		break;
		case ETYPE::VARIABLE:
			if (isalnum(c) || c == '_') {
				value << c;
				continue;
			}
		break;
		default:
			warningstream << "Type not implemented: " << etype << PRINT_DEBUG(c, pd);
			return;
		}

		// Unhandled character
		switch (c) {
		case '(':
			pd.parentheses++;
			pd.balance.push(0);
			break;
		case ')':
			pd.parentheses--;
			if (pd.parentheses < 0)
				errorstream << "Unexpected character " << PRINT_DEBUG(c, pd);
			if (pd.balance.top() != 0) // Not 1 because the variable is added later
				errorstream << "Operation balance != 0 " << PRINT_DEBUG(c, pd);
			pd.balance.pop();
			if (etype == ETYPE::NONE)
				pd.balance.top()++;
			break;
		case ';':
			if (pd.parentheses > 0)
				errorstream << "Unclosed parentheses" << PRINT_DEBUG(c, pd);
			if (pd.balance.top() != 0)
				errorstream << "Operation balance != 0 " << PRINT_DEBUG(c, pd);
			return;
		default:
			pd.is->unget();
			break;
		}
		// Cannot do anything with this character. What to do?
		if (etype == ETYPE::NONE)
			continue;
		else
			break;
	}

read_complete:

	const string &text = value.str();
	Element e = {
		.type = etype,
		.flags = 0,
		.priority = (uint16_t)(pd.parentheses << 8),
		.val = {0}
	};

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
	case ETYPE::OPERATION:
		e.priority |= operation->opcode;
		e.val.op = new OpCodeInformation(*operation);
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

	const int8_t balance = pd.balance.top(); // 0
	cout << "Adding " << (int)e.type
		<< ", pos=" << (int)pd.char_pos
		<< ", stack=" << (int)balance
		<< endl;

	if (e.type == ETYPE::OPERATION) {
		if (std::abs(e.val.op->nary) > balance) {
			while (operator_stack.size() > 0) {
				if (e.priority <= operator_stack.back().priority) {
					// a * b +
					// Move from operator stack to element list
					el.push_back(operator_stack.back());
					operator_stack.pop_back();
				} else {
					break;
				}
			}
			// Not enough values. Delay use of operator
			operator_stack.push_back(e);
		} else {
			errorstream << "Is this inline notation? "
				<< "\n\tBalance = " << (int)balance
				<< "\n\tNary = " << (int)e.val.op->nary
				<< PRINT_DEBUG(c, pd);
		}
	} else {
		// Value first, operator afterwards
		el.push_back(e);
	}

	// TODO Put left-over operator stack contents into element list

	if (etype != ETYPE::OPERATION)
		pd.balance.top()++; // +1 for values, -x for operators
	else
		pd.balance.top() += -std::abs(e.val.op->nary) + 1; // Takes N elements, adds 1 result
}
