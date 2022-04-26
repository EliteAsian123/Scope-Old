#include "extern.h"

#define INPUT_BUFFER_SIZE 256

#define mathFunc(f)                    \
	Object a = pop();                  \
	push((Object){                     \
		.type = type(TYPE_DOUBLE),     \
		.v.v_double = f(a.v.v_double), \
	});

static void _print() {
	Object a = pop();
	char* cstr = strToCstr(a.v.v_string);
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

	push((Object){
		.type = type(TYPE_STR),
		.v.v_string = str,
		.referenceId = basicReference,
	});
}

static void _stringToInt() {
	Object a = pop();

	// Convert str to cstr then use atoi
	char* str = strToCstr(a.v.v_string);
	push((Object){
		.type = type(TYPE_INT),
		.v.v_int = atoi(str),
	});
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
	Object a = pop();

	if (a.v.v_int != 0) {
		printf("Program was forced to exit with non-zero exit code `%d`.\n", a.v.v_int);
	}

	exit(a.v.v_int);
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

#undef mathFunc