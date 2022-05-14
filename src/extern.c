#include "extern.h"

#define INPUT_BUFFER_SIZE 256

#define mathFunc(f)                        \
	Value a = getValue(pop());             \
	push(toElem((Value){                   \
		.type = type(TYPE_DOUBLE),         \
		.data._double = f(a.data._double), \
	}));

static void _print() {
	Value a = getValue(pop());
	char* cstr = strToCstr(a.data._string);
	printf("%s", cstr);
	free(cstr);
}

static void _input() {
	// Get the input
	char in[INPUT_BUFFER_SIZE];
	fgets(in, INPUT_BUFFER_SIZE, stdin);

	// Convert to String (also get rid of new line)
	int len = strlen(in) - 1;
	String str;
	str.len = len;
	str.chars = malloc(len);
	memcpy(str.chars, in, len);

	push(toElem((Value){
		.type = type(TYPE_STR),
		.data._string = str,
	}));
}

static void _stringToInt() {
	Value a = getValue(pop());

	// Convert str to cstr then use atoi
	char* str = strToCstr(a.data._string);
	push(toElem((Value){
		.type = type(TYPE_INT),
		.data._int = atoi(str),
	}));
	free(str);
}

static void _sqrt() {
	mathFunc(sqrt);
}

static void _sin() {
	mathFunc(sin);
}

static void _cos() {
	mathFunc(cos);
}

static void _tan() {
	mathFunc(tan);
}

static void _asin() {
	mathFunc(asin);
}

static void _acos() {
	mathFunc(acos);
}

static void _atan() {
	mathFunc(atan);
}

static void _exit() {
	Value a = getValue(pop());

	if (a.data._int != 0) {
		printf("Program was forced to exit with non-zero exit code `%d`.\n", a.data._int);
	}

	exit(a.data._int);
}

const ExternPtr externs[] = {
	_print,
	_input,
	_stringToInt,
	_sqrt,
	_exit,
	_sin,
	_cos,
	_tan,
	_asin,
	_acos,
	_atan,
};
const int externLen = 11;

#undef mathFunc