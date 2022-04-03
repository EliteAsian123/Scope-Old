#ifndef S_BYTECODE_H
#define S_BYTECODE_H

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "extern.h"
#include "internaltypes.h"
#include "types.h"

#define STACK_SIZE 256

typedef struct {
	int inst;
	int scope;
	size_t location;

	TypeInfo type;
	ValueHolder a;
} Inst;

enum Insts {
	LOAD = 1,
	LOADT,
	LOADV,
	LOADA,
	SAVEV,
	RESAVEV,
	SAVEF,
	CALLF,
	ENDF,
	EXTERN,
	APPENDT,
	ARRAYI,
	ARRAYS,
	ARRAYG,
	ARRAYL,
	SWAP,
	NOT,
	AND,
	OR,
	ADD,
	SUB,
	MUL,
	DIV,
	MOD,
	NEG,
	EQ,
	GT,
	LT,
	GTE,
	LTE,
	CSTR,
	GOTO,
	IFN,
	_INSTS_ENUM_LEN
};

Inst* insts;
size_t instsCount;

void initTypeArgs(TypeInfo* i, TypeInfo args[], size_t len);
bool typeInfoEqual(TypeInfo a, TypeInfo b);

void push(StackElem elem);
StackElem pop();

void pushLoc();
void pushSLoc();
int popLoc();
int readLoc();
void pushLoop();
int popLoop();
int readLoop();
void pushArg(char* arg);
char* popArg();

void pushInst(Inst i, int scope);
void setInst(Inst i, int loc, int scope);
void pushInstAt(Inst i, int loc, int scope);

void startMoveBuffer();
void endMoveBuffer();
void putMoveBuffer(int scope);

void bc_init();
void bc_run(bool showByteCode, bool showCount);
void bc_end();

#endif