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
	if (a.args == NULL) {
		return;
	}

	for (size_t i = 0; i < a.argsLen; i++) {
		freeTypeInfo(a.args[i]);
	}

	free(a.args);
}

TypeInfo dupTypeInfo(TypeInfo a) {
	TypeInfo b = a;

	if (b.args != NULL) {
		b.args = malloc(sizeof(TypeInfo) * a.argsLen);
		for (size_t i = 0; i < b.argsLen; i++) {
			b.args[i] = dupTypeInfo(a.args[i]);
		}
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

NamedObject unnamedToNamed(Object obj, char* name, int scope) {
	return (NamedObject){
		.name = name,
		.scope = scope,
		.o = obj,
	};
}

Object objdup(Object obj) {
	Object out = obj;
	out.type = dupTypeInfo(obj.type);

	return out;
}