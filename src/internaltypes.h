#ifndef S_INTERNALTYPES_H
#define S_INTERNALTYPES_H

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define type(...) \
	(TypeInfo) { .id = __VA_ARGS__ }

#define toElem(...) \
	(StackElem) { .elem = __VA_ARGS__, .var = NULL }

#define toVar(...) \
	(StackElem) { .elem = (Value){0}, .var = __VA_ARGS__ }

struct TypeInfo {
	int id;
	struct TypeInfo* args;
	size_t argsLen;
};
typedef struct TypeInfo TypeInfo;

union Data;
typedef struct {
	TypeInfo type;
	Data data;
	int refCount;
	int scope;
	bool fromArgs;
} Value;

typedef struct {
	char* name;
	Value* value;
} Name;

typedef struct {
	Name* var;
	Value elem;
} StackElem;

typedef struct {
	Name* names;
	size_t len;
} NameList;

// ==================== //

typedef struct {
	char* chars;
	size_t len;
} String;

typedef struct {
	Value** arr;
	size_t len;
} Array;

typedef struct {
	Name* members;
} Utility;

typedef struct {
	int location;
	TypeInfo type;
	char** args;
	size_t argsLen;
} FuncPointer;

// ==================== //

typedef union Data {
	void* _ptr;
	int _int;
	float _float;
	long _long;
	double _double;
	Array _array;
	String _string;
	Utility _utility;
} Data;

bool typeInfoEqual(TypeInfo a, TypeInfo b);
void freeTypeInfo(TypeInfo a);
TypeInfo dupTypeInfo(TypeInfo a);

String cstrToStr(const char* cstr);
char* strToCstr(const String str);

Value getValue(StackElem s);
Value* dupVal(Value v);

#endif