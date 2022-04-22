#ifndef S_REFERENCES_H
#define S_REFERENCES_H

#include "internaltypes.h"

extern ReferenceInfo* refs;
extern size_t refsCount;

size_t createReference(ReferenceInfo info);

#endif