#ifndef S_TYPES_H
#define S_TYPES_H

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../bytecode.h"

#define simpleMathOp(t, dataLoc, ...)                              \
	return (Value){                                                \
		.type = type(t),                                           \
		.data.dataLoc = a.data.dataLoc __VA_ARGS__ b.data.dataLoc, \
	};

#define simpleBoolOp(dataLoc, ...)                              \
	return (Value){                                             \
		.type = type(TYPE_BOOL),                                \
		.data._int = a.data.dataLoc __VA_ARGS__ b.data.dataLoc, \
	};

#define simpleStringCast(x, s)                                                   \
	int length = snprintf(NULL, 0, s, x) + 1;                                    \
	char* str = malloc(length);                                                  \
	snprintf(str, length, s, x);                                                 \
	Value out = (Value){.type = type(TYPE_STR), .data._string = cstrToStr(str)}; \
	free(str);                                                                   \
	return out;

#define standardOpSet(name) \
	.opNeg = name##OpNeg,   \
	.opAdd = name##OpAdd,   \
	.opSub = name##OpSub,   \
	.opMul = name##OpMul,   \
	.opDiv = name##OpDiv,   \
	.opEq = name##OpEq,     \
	.opGt = name##OpGt,     \
	.opLt = name##OpLt,     \
	.opGte = name##OpGte,   \
	.opLte = name##OpLte

typedef Value (*SingleOperation)(Value a);
typedef Value (*DoubleOperation)(Value a, Value b);
typedef Value (*CastOperation)(TypeInfo type, Value v);

typedef void (*DisposeFunc)(const TypeInfo type, const Data v);
typedef Data (*DuplicateFunc)(const TypeInfo type, const Data v);
typedef Data (*CreateDefaultFunc)(const TypeInfo type);

typedef struct {
	const char* displayName;
	size_t (*size)(const void* ptr);
	CreateDefaultFunc createDefault;
	DisposeFunc dispose;
	DuplicateFunc duplicate;

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
	TYPE_OBJECT,
	TYPE_INIT_OBJ,
	_TYPES_ENUM_LEN
};

extern const Type types[];

const char* typestr(TypeInfo type);
Data createDefaultType(TypeInfo type);
void* typedup(int id, const void* ptr);
void dispose(Name name);
Value dupValue(Value v);

InitObject createInitObject(ObjectPointer obj);

#endif