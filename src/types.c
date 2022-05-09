#include "types.h"

static ValueHolder createDefaultInt(TypeInfo type) {
	return (ValueHolder){.v_int = 0};
}

static ValueHolder createDefaultStr(TypeInfo type) {
	String str = (String){
		.chars = NULL,
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
		.arr = NULL,
		.len = 0,
	};

	return (ValueHolder){.v_array = a};
}

static ValueHolder createDefaultLong(TypeInfo type) {
	return (ValueHolder){.v_long = 0};
}

static ValueHolder createDefaultDouble(TypeInfo type) {
	return (ValueHolder){.v_double = 0.0};
}

static ValueHolder errorOnDefault(TypeInfo type) {
	fprintf(stderr, "Attempted to create a default value of a type that does not have a default value.\n");
	exit(-1);
}

static void disposeString(const TypeInfo type, ValueHolder v) {
	free(v.v_string.chars);
}

static void disposeArray(const TypeInfo type, ValueHolder v) {
	if (isDisposable(type.args[0].id)) {
		for (int i = 0; i < v.v_array.len; i++) {
			refs[v.v_array.arr[i].referenceId].counter--;
			disposeIfNoRefs(v.v_array.arr[i]);
		}
	}

	free(v.v_array.arr);
}

static void disposeUtility(const TypeInfo type, ValueHolder v) {
	freeObjectList(v.v_utility.o);
	free(v.v_utility.o);
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

ValueHolder createDefaultType(TypeInfo type) {
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

		types[name.value->type.id].dispose(name.value);
	}
}

bool isDisposable(int id) {
	return types[id].disposable;
}