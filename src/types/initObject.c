#include "types.h"

static void disposeInitObject(const TypeInfo type, Data v) {
	free(v._initObject.members->names);
	free(v._initObject.members);
}