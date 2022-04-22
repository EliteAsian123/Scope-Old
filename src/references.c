#include "references.h"

ReferenceInfo* refs = NULL;
size_t refsCount = 0;

size_t createReference(ReferenceInfo info) {
	refsCount++;
	refs = realloc(refs, sizeof(ReferenceInfo) * refsCount);
	refs[refsCount - 1] = info;
	return refsCount - 1;
}