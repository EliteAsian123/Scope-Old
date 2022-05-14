#include "types.h"

static Value boolOpNot(Value a) {
	return (Value){
		.type = type(TYPE_BOOL),
		.data._int = !a.data._int,
	};
}

static Value boolOpAnd(Value a, Value b) {
	return (Value){
		.type = type(TYPE_BOOL),
		.data._int = a.data._int && b.data._int,
	};
}

static Value boolOpOr(Value a, Value b) {
	return (Value){
		.type = type(TYPE_BOOL),
		.data._int = a.data._int || b.data._int,
	};
}

static Value boolCastTo(TypeInfo type, Value v) {
	if (type.id == TYPE_INT) {
		return (Value){
			.type = type(TYPE_INT),
			.data._int = v.data._int,
		};
	} else if (type.id == TYPE_LONG) {
		return (Value){
			.type = type(TYPE_LONG),
			.data._long = (long) v.data._int,
		};
	} else if (type.id == TYPE_STR) {
		if (v.data._int) {
			return (Value){
				.type = type(TYPE_STR),
				.data._string = cstrToStr("true"),
			};
		} else {
			return (Value){
				.type = type(TYPE_STR),
				.data._string = cstrToStr("false"),
			};
		}
	}
}