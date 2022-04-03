#include "types.h"

static size_t voidSize(const void* ptr) {
	return 0;
}

static size_t intSize(const void* ptr) {
	return sizeof(int);
}

static size_t stringSize(const void* ptr) {
	return sizeof(String);
}

static size_t floatSize(const void* ptr) {
	return sizeof(float);
}

static size_t arraySize(const void* ptr) {
	return sizeof(Array);
}

static void* intFromStackElem(StackElem e) {
	void* ptr = malloc(sizeof(int));
	*(int*) ptr = e.v.v_int;
	return ptr;
}

static void* strFromStackElem(StackElem e) {
	String* str = malloc(sizeof(String));
	*str = (String){
		.len = e.v.v_string.len,
	};

	str->chars = malloc(str->len);
	memcpy(str->chars, e.v.v_string.chars, str->len);

	return str;
}

static void* floatFromStackElem(StackElem e) {
	void* ptr = malloc(sizeof(float));
	*(float*) ptr = e.v.v_int;
	return ptr;
}

static void* arrayFromStackElem(StackElem e) {
	Array* a = malloc(sizeof(Array));
	*a = (Array){
		.len = e.v.v_array.len,
		.arr = malloc(e.v.v_array.len * sizeof(ValueHolder)),
	};

	TypeInfo type = e.type.args[0];

	for (size_t i = 0; i < a->len; i++) {
		a->arr[i] = ptrToValueHolder(type, &e.v.v_array.arr[i]);
	}

	return a;
}

static ValueHolder intToValueHolder(TypeInfo type, const void* ptr) {
	ValueHolder v;
	v.v_int = *(int*) ptr;
	return v;
}

static ValueHolder strToValueHolder(TypeInfo type, const void* ptr) {
	String str = *(String*) ptr;

	ValueHolder v;
	v.v_string.len = v.v_string.len;
	memcpy(v.v_string.chars, str.chars, str.len);

	return v;
}

static ValueHolder floatToValueHolder(TypeInfo type, const void* ptr) {
	ValueHolder v;
	v.v_int = *(float*) ptr;
	return v;
}

static ValueHolder arrayToValueHolder(TypeInfo type, const void* ptr) {
	Array a = *(Array*) ptr;

	ValueHolder v;
	v.v_array.len = a.len;
	v.v_array.arr = malloc(a.len * sizeof(ValueHolder));

	TypeInfo elementType = type.args[0];

	for (size_t i = 0; i < a.len; i++) {
		v.v_array.arr[i] = ptrToValueHolder(elementType, &a.arr[i]);
	}

	return v;
}

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

const Type types[] = {
	{
		.displayName = "void",
		.size = voidSize,
	},
	{
		.displayName = "int",
		.size = intSize,
		.fromStackElem = intFromStackElem,
		.toValueHolder = intToValueHolder,
		.createDefault = createDefaultInt,
	},
	{
		.displayName = "bool",
		.size = intSize,
		.fromStackElem = intFromStackElem,
		.toValueHolder = intToValueHolder,
		.createDefault = createDefaultInt,
	},
	{
		.displayName = "string",
		.size = stringSize,
		.fromStackElem = strFromStackElem,
		.toValueHolder = strToValueHolder,
		.createDefault = createDefaultStr,
	},
	{
		.displayName = "float",
		.size = floatSize,
		.fromStackElem = floatFromStackElem,
		.toValueHolder = floatToValueHolder,
		.createDefault = createDefaultFloat,
	},
	{
		.displayName = "function(?,...)",
		.size = intSize,
		.fromStackElem = intFromStackElem,
		.toValueHolder = intToValueHolder,
		.createDefault = createDefaultFunc,
	},
	{
		.displayName = "array(?)",
		.size = arraySize,
		.fromStackElem = arrayFromStackElem,
		.toValueHolder = arrayToValueHolder,
		.createDefault = createDefaultArray,
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

void* stackElemToPtr(StackElem e) {
	return types[e.type.id].fromStackElem(e);
}

StackElem ptrToStackElem(TypeInfo type, const void* ptr) {
	StackElem e;
	e.type = dupTypeInfo(type);
	e.v = types[type.id].toValueHolder(type, ptr);
	return e;
}

ValueHolder ptrToValueHolder(TypeInfo type, const void* ptr) {
	return types[type.id].toValueHolder(type, ptr);
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