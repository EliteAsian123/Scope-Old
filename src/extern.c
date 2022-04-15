#include "extern.h"

#define INPUT_BUFFER_SIZE 256

static void print() {
	Object a = pop();
	char* cstr = strToCstr(a.v.v_string);
	printf("%s", cstr);
	free(cstr);
}

static void input() {
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

static void stringToInt() {
	Object a = pop();

	// Convert str to cstr then use atoi
	char* str = strToCstr(a.v.v_string);
	push((Object){
		.type = type(TYPE_INT),
		.v.v_int = atoi(str),
	});
	free(str);
}

static void floatSqrt() {
	Object a = pop();

	push((Object){
		.type = type(TYPE_FLOAT),
		.v.v_float = sqrtf(a.v.v_float),
	});
}

static void externExit() {
	Object a = pop();

	if (a.v.v_int != 0) {
		printf("Program was forced to exit with non-zero exit code `%d`.\n", a.v.v_int);
	}

	exit(a.v.v_int);
}

const ExternPtr externs[] = {
	print,
	input,
	stringToInt,
	floatSqrt,
	externExit,
};