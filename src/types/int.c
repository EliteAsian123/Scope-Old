#include "types.h"

static Data createDefaultInt(TypeInfo type) {
	return (Data){._int = 0};
}

static Value intOpNeg(Value a) {
	return (Value){
		.type = type(TYPE_INT),
		.data._int = -a.data._int,
	};
}

static Value intOpAdd(Value a, Value b) {
	simpleMathOp(TYPE_INT, _int, +);
}

static Value intOpSub(Value a, Value b) {
	simpleMathOp(TYPE_INT, _int, -);
}

static Value intOpMul(Value a, Value b) {
	simpleMathOp(TYPE_INT, _int, *);
}

static Value intOpDiv(Value a, Value b) {
	simpleMathOp(TYPE_INT, _int, /);
}

static Value intOpMod(Value a, Value b) {
	int answer = a.data._int % b.data._int;

	// C's modulo is weird.
	if (answer < 0) {
		answer += b.data._int;
	}

	return (Value){
		.type = type(TYPE_INT),
		.data._int = answer,
	};
}

// https://stackoverflow.com/questions/29787310/does-pow-work-for-int-data-type-in-c
static Value intOpPow(Value a, Value b) {
	if (b.data._int < 0) {
		fprintf(stderr, "Math Error: Attempted to raise an integer to a negative power.\n");
		exit(-1);
	}

	int result = 1;
	int exp = b.data._int;
	int base = a.data._int;
	while (exp != 0) {
		if (exp % 2) {
			result *= base;
		}

		exp /= 2;
		base *= base;
	}

	return (Value){
		.type = type(TYPE_INT),
		.data._int = result,
	};
}

static Value intOpEq(Value a, Value b) {
	simpleBoolOp(_int, ==);
}

static Value intOpGt(Value a, Value b) {
	simpleBoolOp(_int, >);
}

static Value intOpLt(Value a, Value b) {
	simpleBoolOp(_int, <);
}

static Value intOpGte(Value a, Value b) {
	simpleBoolOp(_int, >=);
}

static Value intOpLte(Value a, Value b) {
	simpleBoolOp(_int, <=);
}

static Value intCastTo(TypeInfo type, Value v) {
	if (type.id == TYPE_FLOAT) {
		return (Value){
			.type = type(TYPE_FLOAT),
			.data._float = (float) v.data._int,
		};
	} else if (type.id == TYPE_LONG) {
		return (Value){
			.type = type(TYPE_LONG),
			.data._long = (long) v.data._int,
		};
	} else if (type.id == TYPE_DOUBLE) {
		return (Value){
			.type = type(TYPE_DOUBLE),
			.data._double = (double) v.data._int,
		};
	} else if (type.id == TYPE_STR) {
		simpleStringCast(v.data._int, "%d");
	}
}