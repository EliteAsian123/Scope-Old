#include "types.h"

static void disposeInitObject(const TypeInfo type, Data v) {
	free(v._initObject.members->names);
	free(v._initObject.members);
}

static Data createDefaultInitObject(TypeInfo type) {
	return (Data){
		._initObject = createInitObject(objects[type.objectIndex]),
	};
}

static Data initObjectDuplicate(const TypeInfo type, const Data v) {
	Data out = v;

	int size = sizeof(Value*) * out._array.len;
	out._array.arr = malloc(size);
	memcpy(out._array.arr, v._array.arr, size);

	return out;
}