#include "types.h"

static void disposeUtility(const TypeInfo type, Data v) {
	free(v._utility.members->names);
}