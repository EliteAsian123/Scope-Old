#include "references.h"

ReferenceInfo* refs = NULL;
size_t refsCount = 0;

size_t createReference(ReferenceInfo info) {
	refsCount++;
	refs = realloc(refs, sizeof(ReferenceInfo) * refsCount);
	refs[refsCount - 1] = info;
	return refsCount - 1;
}

bool hasNoReferences(size_t id) {
	return refs[id].counter <= 0;
}

bool disposeIfNoRefs(Object obj) {
	if (hasNoReferences(obj.referenceId)) {
		if (showDisposeInfo) {
			printf("Disposing: `%ld`\n", obj.referenceId);
		}

		dispose(obj.type, obj.v);

		return true;
	}

	return false;
}