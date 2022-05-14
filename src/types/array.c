#include "types.h"

static Data createDefaultArray(TypeInfo type) {
	Array a = (Array){
		.arr = NULL,
		.len = 0,
	};

	return (Data){._array = a};
}

static void disposeArray(const TypeInfo type, Data v) {
	// if (isDisposable(type.args[0].id)) {
	// 	for (int i = 0; i < v._array.len; i++) {
	// 		refs[v._array.arr[i].referenceId].counter--;
	// 		disposeIfNoRefs(v._array.arr[i]);
	// 	}
	// }

	// free(v._array.arr);
}