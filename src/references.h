#ifndef S_REFERENCES_H
#define S_REFERENCES_H

#include "flags.h"
#include "internaltypes.h"
#include "types.h"

typedef struct {
	int counter;
} ReferenceInfo;

#define basicReference createReference((ReferenceInfo){ \
	.counter = 0,                                       \
})

extern ReferenceInfo* refs;
extern size_t refsCount;

size_t createReference(ReferenceInfo info);
bool hasNoReferences(size_t id);
bool disposeIfNoRefs(Object obj);

#endif