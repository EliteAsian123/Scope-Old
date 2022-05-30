#include "extern.h"

#define INPUT_BUFFER_SIZE 256

#define mathFunc(f)                        \
	Value a = getValue(pop());             \
	push(toElem((Value){                   \
		.type = type(TYPE_DOUBLE),         \
		.data._double = f(a.data._double), \
	}));

static void e_print() {
	Value a = getValue(pop());
	char* cstr = strToCstr(a.data._string);
	printf("%s", cstr);
	free(cstr);
}

static void e_input() {
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

static void e_stringToInt() {
	Value a = getValue(pop());

	// Convert str to cstr then use atoi
	char* str = strToCstr(a.data._string);
	push(toElem((Value){
		.type = type(TYPE_INT),
		.data._int = atoi(str),
	}));
	free(str);
}

static void e_sqrt() {
	mathFunc(sqrt);
}

static void e_sin() {
	mathFunc(sin);
}

static void e_cos() {
	mathFunc(cos);
}

static void e_tan() {
	mathFunc(tan);
}

static void e_asin() {
	mathFunc(asin);
}

static void e_acos() {
	mathFunc(acos);
}

static void e_atan() {
	mathFunc(atan);
}

static void e_exit() {
	Value a = getValue(pop());

	if (a.data._int != 0) {
		printf("Program was forced to exit with non-zero exit code `%d`.\n", a.data._int);
	}

	exit(a.data._int);
}

static void e_sleep() {
	Value a = getValue(pop());
	usleep(a.data._long);
}

const ExternPtr externs[] = {
	e_print,
	e_input,
	e_stringToInt,
	e_sqrt,
	e_exit,
	e_sin,
	e_cos,
	e_tan,
	e_asin,
	e_acos,
	e_atan,
	e_sleep,
};
const int externLen = 12;

#undef mathFunc