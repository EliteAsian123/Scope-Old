#include "types.h"

static Data createDefaultInt(TypeInfo type) {
	return (Data){._int = 0};
}

static Data createDefaultStr(TypeInfo type) {
	String str = (String){
		.chars = NULL,
		.len = 0,
	};

	return (Data){._string = str};
}

static Data createDefaultFloat(TypeInfo type) {
	return (Data){._float = 0.0};
}

static Data createDefaultFunc(TypeInfo type) {
	return (Data){._int = -1};
}

static Data createDefaultArray(TypeInfo type) {
	Array a = (Array){
		.arr = NULL,
		.len = 0,
	};

	return (Data){._array = a};
}

static Data createDefaultLong(TypeInfo type) {
	return (Data){._long = 0};
}

static Data createDefaultDouble(TypeInfo type) {
	return (Data){._double = 0.0};
}

static Data errorOnDefault(TypeInfo type) {
	fprintf(stderr, "Attempted to create a default value of a type that does not have a default value.\n");
	exit(-1);
}

static void disposeString(const TypeInfo type, Data v) {
	free(v._string.chars);
}

static void disposeArray(const TypeInfo type, Data v) {
	// if (isDisposable(type.args[0].id)) {
	// 	for (int i = 0; i < v._array.len; i++) {
	// 		refs[v._array.arr[i].referenceId].counter--;
	// 		disposeIfNoRefs(v._array.arr[i]);
	// 	}
	// }

	// free(v._array.arr);
}

static void disposeUtility(const TypeInfo type, Data v) {
	free(v._utility.members.names);
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
	{
		.displayName = "double",
		.createDefault = createDefaultDouble,
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