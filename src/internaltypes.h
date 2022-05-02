#ifndef S_INTERNALTYPES_H
#define S_INTERNALTYPES_H

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define type(...) \
	(TypeInfo) { .id = __VA_ARGS__ }

typedef struct {
	char* chars;
	size_t len;
} String;

struct Object;
typedef struct {
	struct Object* arr;
	size_t len;
} Array;

struct ObjectList;
typedef struct {
	struct ObjectList* o;
} Utility;

typedef union {
	void* v_ptr;
	int v_int;
	float v_float;
	long v_long;
	double v_double;
	Array v_array;
	String v_string;
	Utility v_utility;
} ValueHolder;

struct TypeInfo {
	int id;
	struct TypeInfo* args;
	size_t argsLen;
};
typedef struct TypeInfo TypeInfo;

typedef struct Object {
	size_t referenceId;
	ValueHolder v;

	char* name;
	int scope;
	TypeInfo type;
	bool fromArgs;
} Object;

typedef struct {
	int location;
	TypeInfo type;
	char** args;
	size_t argsLen;
} FuncPointer;

typedef struct ObjectList {
	Object* vars;
	size_t varsCount;
} ObjectList;

bool typeInfoEqual(TypeInfo a, TypeInfo b);
void freeTypeInfo(TypeInfo a);
TypeInfo dupTypeInfo(TypeInfo a);
String cstrToStr(const char* cstr);
char* strToCstr(const String str);
Object objdup(Object obj);
void freeObjectList(ObjectList* o);

#endif