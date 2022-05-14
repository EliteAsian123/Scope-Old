#include "types.h"

#ifndef S_TYPES_C
#define S_TYPES_C
#include "array.c"
#include "bool.c"
#include "double.c"
#include "float.c"
#include "function.c"
#include "int.c"
#include "long.c"
#include "string.c"
#include "utility.c"
#endif

static Data errorOnDefault(TypeInfo type) {
	fprintf(stderr, "Attempted to create a default value of a type that does not have a default value.\n");
	exit(-1);
}

const Type types[] = {
	{
		.displayName = "void",
	},
	{
		.displayName = "int",
		.createDefault = createDefaultInt,
		.castTo = intCastTo,
		standardOpSet(int),
		.opMod = intOpMod,
	},
	{
		.displayName = "bool",
		.createDefault = createDefaultInt,
		.castTo = boolCastTo,
		.opNot = boolOpNot,
		.opAnd = boolOpAnd,
		.opOr = boolOpOr,
	},
	{
		.displayName = "string",
		.createDefault = createDefaultString,
		.disposable = true,
		.dispose = disposeString,
		.opEq = stringOpEq,
		.opAdd = stringOpAdd,
	},
	{
		.displayName = "float",
		.createDefault = createDefaultFloat,
		.castTo = floatCastTo,
		standardOpSet(float),
	},
	{
		.displayName = "function(?,...)",
		.createDefault = createDefaultFunction,
	},
	{
		.displayName = "array(?)",
		.createDefault = createDefaultArray,
		.disposable = true,
		.dispose = disposeArray,
	},
	{
		.displayName = "long",
		.createDefault = createDefaultLong,
		.castTo = longCastTo,
		standardOpSet(long),
		.opMod = longOpMod,
	},
	{
		.displayName = "double",
		.createDefault = createDefaultDouble,
		.castTo = doubleCastTo,
		standardOpSet(double),
	},
	{
		.displayName = "utility",
		.createDefault = errorOnDefault,
		.disposable = true,
		.dispose = disposeUtility,
	},
};
static_assert(sizeof(types) / sizeof(Type) == _TYPES_ENUM_LEN, "Update enum or type array.");

const char* typestr(int id) {
	switch (id) {
		case TYPE_UNKNOWN:
			return "unknown";
		default:
			return types[id].displayName;
	}
}

Data createDefaultType(TypeInfo type) {
	return types[type.id].createDefault(type);
}

void* typedup(int id, const void* ptr) {
	size_t size = types[id].size(ptr);

	void* p = malloc(size);
	memcpy(p, ptr, size);
	return p;
}

void dispose(Name name) {
	if (types[name.value->type.id].disposable) {
		if (showDisposeInfo) {
			printf("Disposing: `%s`\n", name.name);
		}

		types[name.value->type.id].dispose(name.value->type, name.value->data);
	}
}

bool isDisposable(int id) {
	return types[id].disposable;
}