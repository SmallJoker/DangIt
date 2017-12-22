# DangIt - The scripting language
This is a project to learn C++. The target is to make a fast scripting language that has similarities to Lua.
It will not compile, even when using CMakeLists.txt because these are just bones. Do not use plain bones to feed dogs.

License: BSD 3-Clause (ALL YOUR CODE ARE BELONG TO US)

### Targets

- Fast as possible (JIT functions planned)
- Code well formatted (Linux kernel code style guidelines)
- Ability to embed into applications

### Example code
```
int myval = 22;
string myval = "blah";

int combine(string a, string b, int count)
{
	if (a == b)
		return 0xC0FFEE;

	if strlen(a) > #b { // # and strlen() is the same function
		exception("Can not do that stuff");
	}

	string result; // Can not be null, only empty
	for (int x = 0; x < count; x++)
		result = result + a + b; // Combine string

	return #result;
}

int len = combine(myval, ".png ", 3);
```