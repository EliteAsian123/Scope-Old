#ifndef S_EXTERN_H
#define S_EXTERN_H

#include "bytecode.h"

typedef void (*ExternPtr)();

extern const ExternPtr externs[];

#endif