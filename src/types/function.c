#include "types.h"

static Data createDefaultFunction(TypeInfo type) {
	return (Data){._func = (FuncPointer){.location = -1}};
}

static Value functionOpEq(Value a, Value b) {
	// TODO: fix?
	return (Value){
		.type = type(TYPE_BOOL),
		.data._int = a.data._func.location == b.data._func.location,
	};
}