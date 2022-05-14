#ifndef S_TYPES_H
#define S_TYPES_H

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "flags.h"
#include "internaltypes.h"

typedef Value (*SingleOperation)(Value a);
typedef Value (*DoubleOperation)(Value a, Value b);
typedef Value (*CastOperation)(TypeInfo type, Value v);

typedef struct {
	const char* displayName;
	size_t (*size)(const void* ptr);
	Data (*createDefault)(TypeInfo type);
	bool disposable;
	void (*dispose)(TypeInfo type, Data v);

	CastOperation castTo;
	SingleOperation opNot;
	SingleOperation opNeg;
	DoubleOperation opAnd;
	DoubleOperation opOr;
	DoubleOperation opAdd;
	DoubleOperation opSub;
	DoubleOperation opMul;
	DoubleOperation opDiv;
	DoubleOperation opMod;
	DoubleOperation opPow;
	DoubleOperation opEq;
	DoubleOperation opGt;
	DoubleOperation opLt;
	DoubleOperation opGte;
	DoubleOperation opLte;
} Type;

enum Types {
	TYPE_UNKNOWN = -1,
	TYPE_VOID = 0,
	TYPE_INT = 1,
	TYPE_BOOL,
	TYPE_STR,
	TYPE_FLOAT,
	TYPE_FUNC,
	TYPE_ARRAY,
	TYPE_LONG,
	TYPE_DOUBLE,
	TYPE_UTIL,
	_TYPES_ENUM_LEN
};

extern const Type types[];

const char* typestr(int id);
Data createDefaultType(TypeInfo type);
void* typedup(int id, const void* ptr);
void dispose(Name name);
bool isDisposable(int id);

#endif