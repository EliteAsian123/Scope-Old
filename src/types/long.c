#include "types.h"

static Data createDefaultLong(TypeInfo type) {
	return (Data){._long = 0};
}

static Value longOpNeg(Value a) {
	return (Value){
		.type = type(TYPE_LONG),
		.data._long = -a.data._long,
	};
}

static Value longOpAdd(Value a, Value b) {
	simpleMathOp(TYPE_LONG, _long, +);
}

static Value longOpSub(Value a, Value b) {
	simpleMathOp(TYPE_LONG, _long, -);
}

static Value longOpMul(Value a, Value b) {
	simpleMathOp(TYPE_LONG, _long, *);
}

static Value longOpDiv(Value a, Value b) {
	simpleMathOp(TYPE_LONG, _long, /);
}

static Value longOpMod(Value a, Value b) {
	long answer = a.data._long % b.data._long;

	// C's modulo is weird.
	if (answer < 0) {
		answer += b.data._long;
	}

	return (Value){
		.type = type(TYPE_LONG),
		.data._long = answer,
	};
}

// https://stackoverflow.com/questions/29787310/does-pow-work-for-int-data-type-in-c
static Value longOpPow(Value a, Value b) {
	if (b.data._long < 0) {
		fprintf(stderr, "Math Error: Attempted to raise an integer to a negative power.\n");
		exit(-1);
	}

	long result = 1;
	long exp = b.data._long;
	long base = a.data._long;
	while (exp != 0) {
		if (exp % 2 != 0) {
			result *= base;
		}

		exp /= 2;
		base *= base;
	}

	return (Value){
		.type = type(TYPE_LONG),
		.data._long = result,
	};
}

static Value longOpEq(Value a, Value b) {
	simpleBoolOp(_long, ==);
}

static Value longOpGt(Value a, Value b) {
	simpleBoolOp(_long, >);
}

static Value longOpLt(Value a, Value b) {
	simpleBoolOp(_long, <);
}

static Value longOpGte(Value a, Value b) {
	simpleBoolOp(_long, >=);
}

static Value longOpLte(Value a, Value b) {
	simpleBoolOp(_long, <=);
}

static Value longCastTo(TypeInfo type, Value v) {
	if (type.id == TYPE_FLOAT) {
		return (Value){
			.type = type(TYPE_FLOAT),
			.data._float = (float) v.data._long,
		};
	} else if (type.id == TYPE_INT) {
		return (Value){
			.type = type(TYPE_INT),
			.data._int = (int) v.data._long,
		};
	} else if (type.id == TYPE_DOUBLE) {
		return (Value){
			.type = type(TYPE_DOUBLE),
			.data._double = (double) v.data._long,
		};
	} else if (type.id == TYPE_STR) {
		simpleStringCast(v.data._long, "%ld");
	}
}