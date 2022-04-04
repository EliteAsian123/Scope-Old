#include "internaltypes.h"

bool typeInfoEqual(TypeInfo a, TypeInfo b) {
	if (a.id != b.id) {
		return false;
	}

	if (a.argsLen != b.argsLen) {
		return false;
	}

	for (size_t i = 0; i < a.argsLen; i++) {
		if (!typeInfoEqual(a.args[i], b.args[i])) {
			return false;
		}
	}

	return true;
}

void freeTypeInfo(TypeInfo a) {
	if (a.args == NULL || a.argsLen <= 0) {
		return;
	}

	for (size_t i = 0; i < a.argsLen; i++) {
		freeTypeInfo(a.args[i]);
	}
}

TypeInfo dupTypeInfo(TypeInfo a) {
	TypeInfo b = a;
	for (size_t i = 0; i < b.argsLen; i++) {
		b.args[i] = dupTypeInfo(a.args[i]);
	}
	return b;
}

String cstrToStr(const char* cstr) {
	String str = {
		.len = strlen(cstr),
	};

	str.chars = malloc(str.len);
	memcpy(str.chars, cstr, str.len);

	return str;
}

char* strToCstr(const String str) {
	char* cstr = malloc(str.len + 1);
	memcpy(cstr, str.chars, str.len);
	cstr[str.len] = '\0';

	return cstr;
}

StackElem objectToStackElem(Object obj) {
	return (StackElem){
		.type = obj.type,
		.v = obj.v,
	};
}

Object stackElemToObject(StackElem elem) {
	return (Object){
		.type = elem.type,
		.v = elem.v,
	};
}