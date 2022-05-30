#ifndef S_EXTERN_H
#define S_EXTERN_H

#include <math.h>
#include <unistd.h>

#include "bytecode.h"

typedef void (*ExternPtr)();

extern const ExternPtr externs[];
extern const int externLen;

#endif