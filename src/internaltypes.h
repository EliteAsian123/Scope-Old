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

// ==================== //

typedef struct NameList {
	struct Name* names;
	size_t len;
} NameList;

// ==================== //

typedef struct String {
	char* chars;
	size_t len;
} String;

typedef struct Array {
	struct Value** arr;
	size_t len;
} Array;

typedef struct Utility {
	struct NameList members;
} Utility;

typedef union Data {
	void* _ptr;
	int _int;
	float _float;
	long _long;
	double _double;
	struct Array _array;
	struct String _string;
	struct Utility _utility;
} Data;

// ==================== //

typedef struct TypeInfo {
	int id;
	struct TypeInfo* args;
	size_t argsLen;
} TypeInfo;

typedef struct Value {
	TypeInfo type;
	union Data data;
	int refCount;
	int scope;
	bool fromArgs;
} Value;

typedef struct Name {
	char* name;
	Value* value;
} Name;

typedef struct StackElem {
	Name* var;
	Value elem;
} StackElem;

// ==================== //

typedef struct FuncPointer {
	int location;
	TypeInfo type;
	char** args;
	size_t argsLen;
} FuncPointer;

// ==================== //

bool typeInfoEqual(TypeInfo a, TypeInfo b);
void freeTypeInfo(TypeInfo a);
TypeInfo dupTypeInfo(TypeInfo a);

String cstrToStr(const char* cstr);
char* strToCstr(const String str);
bool stringEqual(String a, String b);

Value getValue(StackElem s);
Value* getValuePtr(StackElem s);
Value* dupValue(Value v);

#endif