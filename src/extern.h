#ifndef S_EXTERN_H
#define S_EXTERN_H

#include <math.h>

#include "bytecode.h"
#include "references.h"

typedef void (*ExternPtr)();

extern const ExternPtr externs[];

#endif