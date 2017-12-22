#include "headers.h"

void simplifyElements(VariableList &vl, ElementList &el)
{
	uint16_t priority_max = 0;
	for (ElementList::iterator it = el.begin(); it != el.end(); it++) {
		if (it->type != ETYPE::OPERATION)
			continue;
		priority_max = max(priority_max, it->priority);
	}

	while (priority_max > 0) {
		uint16_t next_max = 0;
		for (ElementList::iterator it = el.begin(); it != el.end(); it++) {
			if (it->priority == priority_max) {
				
			} else {
				next_max = max(next_max, it->priority);
			}
		}
		priority_max = next_max;
	}
	/* Loop()
		Go through RPN stack and check whether a few actions can be performed
	*/
}
