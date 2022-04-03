#include "extern.h"

#define INPUT_BUFFER_SIZE 256

static void print() {
	StackElem a = pop();
	printf("%s", a.v.v_string.chars);
}

static void input() {
	// Get the input
	char in[INPUT_BUFFER_SIZE];
	fgets(in, INPUT_BUFFER_SIZE, stdin);

	// Get rid of the new line
	int size = strlen(in) - 1;
	char* str = calloc(size, sizeof(char));
	strncpy(str, in, size);
	str[size] = '\0';

	// Push
	push((StackElem){.type = type(TYPE_STR), .v.v_ptr = str});
}

static void stringToInt() {
	StackElem a = pop();
	push((StackElem){.type = type(TYPE_INT), .v.v_int = atoi(a.v.v_ptr)});
}

const ExternPtr externs[] = {
	print,
	input,
	stringToInt,
};