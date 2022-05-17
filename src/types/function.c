#include "types.h"

static Data createDefaultFunction(TypeInfo type) {
	return (Data){._int = -1};
}

static Value functionOpEq(Value a, Value b) {
	simpleBoolOp(_int, ==);
}