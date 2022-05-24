#include "types.h"

static Data createDefaultFloat(TypeInfo type) {
	return (Data){._float = 0.0};
}

static Value floatOpNeg(Value a) {
	return (Value){
		.type = type(TYPE_FLOAT),
		.data._float = -a.data._float,
	};
}

static Value floatOpAdd(Value a, Value b) {
	simpleMathOp(TYPE_FLOAT, _float, +);
}

static Value floatOpSub(Value a, Value b) {
	simpleMathOp(TYPE_FLOAT, _float, -);
}

static Value floatOpMul(Value a, Value b) {
	simpleMathOp(TYPE_FLOAT, _float, *);
}

static Value floatOpDiv(Value a, Value b) {
	simpleMathOp(TYPE_FLOAT, _float, /);
}

static Value floatOpPow(Value a, Value b) {
	return (Value){
		.type = type(TYPE_FLOAT),
		.data._float = powf(a.data._float, b.data._float),
	};
}

static Value floatOpEq(Value a, Value b) {
	fprintf(stderr, "It is recommended not to use the `==` operator on floats. Consider `fApprox` instead.\n");
	simpleBoolOp(_float, ==);
}

static Value floatOpGt(Value a, Value b) {
	simpleBoolOp(_float, >);
}

static Value floatOpLt(Value a, Value b) {
	simpleBoolOp(_float, <);
}

static Value floatOpGte(Value a, Value b) {
	simpleBoolOp(_float, >=);
}

static Value floatOpLte(Value a, Value b) {
	simpleBoolOp(_float, <=);
}

static Value floatCastTo(TypeInfo type, Value v) {
	if (type.id == TYPE_INT) {
		return (Value){
			.type = type(TYPE_INT),
			.data._int = (int) v.data._float,
		};
	} else if (type.id == TYPE_LONG) {
		return (Value){
			.type = type(TYPE_LONG),
			.data._long = (long) v.data._float,
		};
	} else if (type.id == TYPE_DOUBLE) {
		return (Value){
			.type = type(TYPE_DOUBLE),
			.data._double = (double) v.data._float,
		};
	} else if (type.id == TYPE_STR) {
		simpleStringCast(v.data._float, "%f");
	}
}