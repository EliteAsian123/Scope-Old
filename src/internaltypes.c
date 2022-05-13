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

Object objdup(Object obj) {
	Object out = obj;

	if (out.name != NULL) {
		out.name = strdup(out.name);
	}

	out.type = dupTypeInfo(obj.type);

	return out;
}

void freeObjectList(ObjectList* o) {
	for (size_t i = 0; i < o->varsCount; i++) {
		free(o->vars[i].name);
		freeTypeInfo(o->vars[i].type);
	}
	free(o->vars);
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

Value* dupValue(Value v) {
	Value* out = malloc(sizeof(Value));
	*out = v;
	out->type = dupTypeInfo(v.type);

	return out;
}