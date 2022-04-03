#ifndef S_INTERNALTYPES_H
#define S_INTERNALTYPES_H

#include <stdbool.h>
#include <stdlib.h>

#define type(...) \
	(TypeInfo) { .id = __VA_ARGS__ }

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
	TypeInfo type;
	ValueHolder v;
} StackElem;

typedef struct {
	int scope;
	TypeInfo type;
	char* name;
	void* ptr;
	bool fromArgs;
} Object;

typedef struct {
	int location;
	TypeInfo type;
	char** args;
	size_t argsLen;
} FuncPointer;

bool typeInfoEqual(TypeInfo a, TypeInfo b);
void freeTypeInfo(TypeInfo a);
TypeInfo dupTypeInfo(TypeInfo a);

#endif