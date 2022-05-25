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
	InitObject obj = v._initObject;

	obj.members = malloc(sizeof(NameList));
	memcpy(obj.members, v._initObject.members, sizeof(NameList));
	obj.members->names = malloc(sizeof(Name) * obj.members->len);
	for (size_t i = 0; i < obj.members->len; i++) {
		Name original = v._initObject.members->names[i];

		Value* ptr = malloc(sizeof(Value));
		*ptr = dupValue(*original.value);

		obj.members->names[i] = (Name){
			.name = strdup(original.name),
			.value = ptr,
		};
	}

	return (Data){
		._initObject = obj,
	};
}