#include "types.h"

static Data createDefaultString(const TypeInfo type) {
	String str = (String){
		.chars = NULL,
		.len = 0,
	};

	return (Data){._string = str};
}

static void disposeString(const TypeInfo type, Data v) {
	free(v._string.chars);
}

static Data stringDuplicate(const TypeInfo type, Data v) {
	Data ret = v;
	if (ret._string.chars != NULL) {
		ret._string.chars = malloc(v._string.len + 1);
		memcpy(ret._string.chars, v._string.chars, v._string.len + 1);
	}

	return ret;
}

static Value stringOpEq(Value a, Value b) {
	return (Value){
		.type = type(TYPE_BOOL),
		.data._int = stringEqual(a.data._string, b.data._string),
	};
}

static Value stringOpAdd(Value a, Value b) {
	// Create string
	String s = (String){
		.len = a.data._string.len + b.data._string.len,
	};
	s.chars = malloc(s.len);

	// Combine the two strings
	memcpy(s.chars, a.data._string.chars, a.data._string.len);
	memcpy(s.chars + a.data._string.len, b.data._string.chars, b.data._string.len);

	// Push onto stack
	return (Value){
		.type = type(TYPE_STR),
		.data._string = s,
	};
}