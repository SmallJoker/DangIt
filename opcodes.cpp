#include "headers.h"

OpCodeInformation op_lookup[] = {
	{ 0x01, -1, nullptr }, // JMP
	{ 0x02, -1, nullptr }, // IF
	{ 0x10,  2, "="     }, // SETV
// Boolean operations
	{ 0x30,  2, "=="    }, // EQ
	{ 0x31,  2, "!="    }, // NEQ
	{ 0x40,  2, "||"    }, // OR
	{ 0x50,  2, "&&"    }, // AND
	{ 0x60,  2, ">"     }, // GT
	{ 0x61,  2, ">="    }, // GTE
	{ 0x62,  2, "<"     }, // LT
	{ 0x63,  2, "<="    }, // GTE
// Number operations
	{ 0x80,  2, "+"     }, // ADD
	{ 0x81,  2, "-"     }, // SUB
	{ 0x82,  2, "%"     }, // MOD
	{ 0x83,  2, "/"     }, // DIV
	{ 0x84,  2, "*"     }, // MUL
// Unary
	{ 0xC0, -1, "!"     }, // NOT
	{ 0xC1, -1, "~"     }, // INV
	{ 0xC2, -1, "-"     }, // NEG
	{ 0xC3, -1, "#"     }, // COUNTOF
	{ 0xC4,  1, nullptr }, // INDEXOF
	{ 0xC5,  1, nullptr }, // CALL
	{ 0xC6,  1, "++"    }, // INC
	{ 0xC7,  1, "--"    }, // DEC
	{ 0xC8,  1, "!"     }, // Factorial
};
