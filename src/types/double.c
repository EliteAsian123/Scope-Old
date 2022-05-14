#include "types.h"

static Data createDefaultDouble(TypeInfo type) {
	return (Data){._double = 0.0};
}

static Value doubleOpNeg(Value a) {
	return (Value){
		.type = type(TYPE_DOUBLE),
		.data._double = -a.data._double,
	};
}

static Value doubleOpAdd(Value a, Value b) {
	simpleMathOp(TYPE_DOUBLE, _double, +);
}

static Value doubleOpSub(Value a, Value b) {
	simpleMathOp(TYPE_DOUBLE, _double, -);
}

static Value doubleOpMul(Value a, Value b) {
	simpleMathOp(TYPE_DOUBLE, _double, *);
}

static Value doubleOpDiv(Value a, Value b) {
	simpleMathOp(TYPE_DOUBLE, _double, /);
}

static Value doubleOpEq(Value a, Value b) {
	fprintf(stderr, "It is recommended not to use the `==` operator on doubles. Consider `dApprox` instead.\n");
	simpleBoolOp(_double, ==);
}

static Value doubleOpGt(Value a, Value b) {
	simpleBoolOp(_double, >);
}

static Value doubleOpLt(Value a, Value b) {
	simpleBoolOp(_double, <);
}

static Value doubleOpGte(Value a, Value b) {
	simpleBoolOp(_double, >=);
}

static Value doubleOpLte(Value a, Value b) {
	simpleBoolOp(_double, <=);
}

static Value doubleCastTo(TypeInfo type, Value v) {
	if (type.id == TYPE_INT) {
		return (Value){
			.type = type(TYPE_INT),
			.data._int = (int) v.data._double,
		};
	} else if (type.id == TYPE_LONG) {
		return (Value){
			.type = type(TYPE_LONG),
			.data._long = (long) v.data._double,
		};
	} else if (type.id == TYPE_FLOAT) {
		return (Value){
			.type = type(TYPE_FLOAT),
			.data._float = (float) v.data._double,
		};
	} else if (type.id == TYPE_STR) {
		simpleStringCast(v.data._double, "%lf");
	}
}