#ifndef S_TYPES_H
#define S_TYPES_H

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "internaltypes.h"
#include "references.h"

typedef struct {
	const char* displayName;
	size_t (*size)(const void* ptr);
	ValueHolder (*createDefault)(TypeInfo type);
	bool disposable;
	void (*dispose)(TypeInfo type, ValueHolder v);
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
	_TYPES_ENUM_LEN
};

extern const Type types[];

const char* typestr(int id);
ValueHolder createDefaultType(TypeInfo type);
void* typedup(int id, const void* ptr);
void dispose(TypeInfo type, ValueHolder v);
bool isDisposable(int id);

#endif