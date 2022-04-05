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

	push((Object){.type = type(TYPE_STR), .v.v_string = str});
}

static void stringToInt() {
	Object a = pop();

	// Convert str to cstr then use atoi
	char* str = strToCstr(a.v.v_string);
	push((Object){.type = type(TYPE_INT), .v.v_int = atoi(str)});
	free(str);
}

const ExternPtr externs[] = {
	print,
	input,
	stringToInt,
};