#include "internaltypes.h"

bool typeInfoEqual(TypeInfo a, TypeInfo b) {
	if (a.id != b.id) {
		return false;
	}

	if (a.objectIndex != b.objectIndex) {
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

bool stringEqual(String a, String b) {
	if (a.len != b.len) {
		return false;
	}

	for (size_t i = 0; i < a.len; i++) {
		if (a.chars[i] != b.chars[i]) {
			return false;
		}
	}

	return true;
}

Value getValue(StackElem s) {
	if (s.var == NULL) {
		return s.elem;
	} else {
		return *s.var->value;
	}
}

Value* getValuePtr(StackElem s) {
	if (s.var == NULL) {
		Value* ptr = malloc(sizeof(Value));
		*ptr = s.elem;
		return ptr;
	} else {
		return s.var->value;
	}
}