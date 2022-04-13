#include "types.h"

static ValueHolder createDefaultInt(TypeInfo type) {
	return (ValueHolder){.v_int = 0};
}

static ValueHolder createDefaultStr(TypeInfo type) {
	String str = (String){
		.chars = malloc(0),
		.len = 0,
	};

	return (ValueHolder){.v_string = str};
}

static ValueHolder createDefaultFloat(TypeInfo type) {
	return (ValueHolder){.v_float = 0.0};
}

static ValueHolder createDefaultFunc(TypeInfo type) {
	return (ValueHolder){.v_int = -1};
}

static ValueHolder createDefaultArray(TypeInfo type) {
	Array a = (Array){
		.arr = malloc(0),
		.len = 0,
	};

	return (ValueHolder){.v_array = a};
}

static ValueHolder createDefaultLong(TypeInfo type) {
	return (ValueHolder){.v_long = 0};
}

static void disposeString(const TypeInfo type, ValueHolder v) {
	free(v.v_string.chars);
}

static void disposeArray(const TypeInfo type, ValueHolder v) {
	free(v.v_array.arr);
}

const Type types[] = {
	{
		.displayName = "void",
	},
	{
		.displayName = "int",
		.createDefault = createDefaultInt,
	},
	{
		.displayName = "bool",
		.createDefault = createDefaultInt,
	},
	{
		.displayName = "string",
		.createDefault = createDefaultStr,
		.disposable = true,
		.dispose = disposeString,
	},
	{
		.displayName = "float",
		.createDefault = createDefaultFloat,
	},
	{
		.displayName = "function(?,...)",
		.createDefault = createDefaultFunc,
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

ValueHolder createDefaultType(TypeInfo type) {
	return types[type.id].createDefault(type);
}

void* typedup(int id, const void* ptr) {
	size_t size = types[id].size(ptr);

	void* p = malloc(size);
	memcpy(p, ptr, size);
	return p;
}

void dispose(const TypeInfo type, ValueHolder v) {
	if (types[type.id].disposable) {
		types[type.id].dispose(type, v);
	}
}

bool isDisposable(int id) {
	return types[id].disposable;
}