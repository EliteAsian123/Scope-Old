#ifndef S_INTERNALTYPES_H
#define S_INTERNALTYPES_H

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define type(...) \
	(TypeInfo) { .id = __VA_ARGS__ }

#define basicReference createReference((ReferenceInfo){ \
	.counter = 0,                                       \
})

typedef struct {
	char* chars;
	size_t len;
} String;

union ValueHolder;
typedef struct {
	union ValueHolder* arr;
	size_t len;
} Array;

typedef union ValueHolder {
	void* v_ptr;
	int v_int;
	float v_float;
	Array v_array;
	String v_string;
} ValueHolder;

struct TypeInfo {
	int id;
	struct TypeInfo* args;
	size_t argsLen;
};
typedef struct TypeInfo TypeInfo;

typedef struct {
	size_t referenceId;
	TypeInfo type;
	ValueHolder v;
	bool fromArgs;
} Object;

typedef struct {
	char* name;
	int scope;
	Object o;
} NamedObject;

typedef struct {
	int location;
	TypeInfo type;
	char** args;
	size_t argsLen;
} FuncPointer;

typedef struct {
	int counter;
} ReferenceInfo;

bool typeInfoEqual(TypeInfo a, TypeInfo b);
void freeTypeInfo(TypeInfo a);
TypeInfo dupTypeInfo(TypeInfo a);
String cstrToStr(const char* cstr);
char* strToCstr(const String str);
NamedObject unnamedToNamed(Object obj, char* name, int scope);
Object objdup(Object obj);

#endif