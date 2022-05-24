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

static Data errorOnDefault(const TypeInfo type) {
	fprintf(stderr, "Attempted to create a default value of a type that does not have a default value.\n");
	exit(-1);
}

static Data noDuplicate(const TypeInfo type, const Data v) {
	return v;
}

const Type types[] = {
	{
		.displayName = "void",
	},
	{
		.displayName = "int",
		.createDefault = createDefaultInt,
		.duplicate = noDuplicate,
		.castTo = intCastTo,
		standardOpSet(int),
		.opMod = intOpMod,
		.opPow = intOpPow,
	},
	{
		.displayName = "bool",
		.createDefault = createDefaultInt,
		.duplicate = noDuplicate,
		.castTo = boolCastTo,
		.opNot = boolOpNot,
		.opAnd = boolOpAnd,
		.opOr = boolOpOr,
	},
	{
		.displayName = "string",
		.createDefault = createDefaultString,
		.duplicate = stringDuplicate,
		.dispose = disposeString,
		.opEq = stringOpEq,
		.opAdd = stringOpAdd,
	},
	{
		.displayName = "float",
		.createDefault = createDefaultFloat,
		.duplicate = noDuplicate,
		.castTo = floatCastTo,
		standardOpSet(float),
		.opPow = floatOpPow,
	},
	{
		.displayName = "function(?,...)",
		.createDefault = createDefaultFunction,
		.duplicate = noDuplicate,
		.opEq = functionOpEq,
	},
	{
		.displayName = "array(?)",
		.createDefault = createDefaultArray,
		.duplicate = arrayDuplicate,
		.dispose = disposeArray,
	},
	{
		.displayName = "long",
		.createDefault = createDefaultLong,
		.duplicate = noDuplicate,
		.castTo = longCastTo,
		standardOpSet(long),
		.opMod = longOpMod,
		.opPow = longOpPow,
	},
	{
		.displayName = "double",
		.createDefault = createDefaultDouble,
		.duplicate = noDuplicate,
		.castTo = doubleCastTo,
		standardOpSet(double),
		.opPow = doubleOpPow,
	},
	{
		.displayName = "utility",
		.createDefault = errorOnDefault,
		.duplicate = noDuplicate,
		.dispose = disposeUtility,
	},
};
static_assert(sizeof(types) / sizeof(Type) == _TYPES_ENUM_LEN, "Update enum or type array.");

const char* typestr(TypeInfo type) {
	char* o;

	switch (type.id) {
		case TYPE_UNKNOWN:
			o = strdup("unknown");
			break;
		default:
			if (type.id > _TYPES_ENUM_LEN) {
				o = strdup("error");
			} else {
				o = strdup(types[type.id].displayName);
			}
			break;
	}

	for (int i = 0; i < type.argsLen; i++) {
		o = strcat(o, " ");
		o = strcat(o, typestr(type.args[i]));
	}

	return o;
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
	DisposeFunc t = types[name.value->type.id].dispose;

	if (t != NULL) {
		t(name.value->type, name.value->data);
	}
}

Value dupValue(Value v) {
	Value out = v;
	out.type = dupTypeInfo(out.type);
	out.refCount = 0;
	out.data = types[out.type.id].duplicate(out.type, out.data);

	return out;
}