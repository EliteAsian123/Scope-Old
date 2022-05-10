#include "bytecode.h"

// TODO: Proper accessors `.`
// TODO: Redo `repeat`
// TODO: Utilities
// TODO: Types
// TODO: size_t for goto and stuff; or relative addresses
// TODO: static checking

#define basicOperation(op)                                                                     \
	b = pop();                                                                                 \
	a = pop();                                                                                 \
	if (a.type.id == TYPE_INT && b.type.id == TYPE_INT) {                                      \
		push((Object){.type = type(TYPE_INT), .v.v_int = a.v.v_int op b.v.v_int});             \
	} else if (a.type.id == TYPE_FLOAT && b.type.id == TYPE_FLOAT) {                           \
		push((Object){.type = type(TYPE_FLOAT), .v.v_float = a.v.v_float op b.v.v_float});     \
	} else if (a.type.id == TYPE_LONG && b.type.id == TYPE_LONG) {                             \
		push((Object){.type = type(TYPE_LONG), .v.v_long = a.v.v_long op b.v.v_long});         \
	} else if (a.type.id == TYPE_DOUBLE && b.type.id == TYPE_DOUBLE) {                         \
		push((Object){.type = type(TYPE_DOUBLE), .v.v_double = a.v.v_double op b.v.v_double}); \
	} else {                                                                                   \
		ierr("Invalid types for `" #op "`.");                                                  \
	}

#define boolOperation(op)                                                                 \
	b = pop();                                                                            \
	a = pop();                                                                            \
	if (a.type.id == TYPE_INT && b.type.id == TYPE_INT) {                                 \
		push((Object){.type = type(TYPE_BOOL), .v.v_int = a.v.v_int op b.v.v_int});       \
	} else if (a.type.id == TYPE_FLOAT && b.type.id == TYPE_FLOAT) {                      \
		push((Object){.type = type(TYPE_BOOL), .v.v_int = a.v.v_float op b.v.v_float});   \
	} else if (a.type.id == TYPE_LONG && b.type.id == TYPE_LONG) {                        \
		push((Object){.type = type(TYPE_BOOL), .v.v_int = a.v.v_long op b.v.v_long});     \
	} else if (a.type.id == TYPE_DOUBLE && b.type.id == TYPE_DOUBLE) {                    \
		push((Object){.type = type(TYPE_BOOL), .v.v_int = a.v.v_double op b.v.v_double}); \
	} else {                                                                              \
		ierr("Invalid types for `" #op "`.");                                             \
	}

#define toStr(x, s)                                                                                      \
	int length = snprintf(NULL, 0, s, x) + 1;                                                            \
	char* str = malloc(length);                                                                          \
	snprintf(str, length, s, x);                                                                         \
	push((Object){.type = type(TYPE_STR), .v.v_string = cstrToStr(str), .referenceId = basicReference}); \
	free(str);

#define ierr(...)                                          \
	fprintf(stderr, "Interpret Error: " __VA_ARGS__ "\n"); \
	exit(-1);

#define iwarn(...) fprintf(stdout, "Interpret Warning: " __VA_ARGS__ "\n");

#define jump(loc) i = loc - 1;

#define curInstBuf instbuffer[instbufferCount - 1]

typedef struct {
	NameList names;
} CallFrame;

typedef struct {
	Inst* insts;
	size_t instsCount;
} InstBuffer;

// Interpret stage
static CallFrame frames[STACK_SIZE];
static size_t framesCount;

// Interpret & Parse stage
static StackElem stack[STACK_SIZE];
static size_t stackCount;

// Parse stage
static int locstack[STACK_SIZE];
static size_t locstackCount;

// Parse stage
static int loopstack[STACK_SIZE];
static size_t loopstackCount;

// Interpret stage
static char* argstack[STACK_SIZE];
static size_t argstackCount;

// Interpret stage
static FuncPointer* funcs;
static size_t funcsCount;

// Parse stage
static InstBuffer instbuffer[STACK_SIZE];
static size_t instbufferCount;
static bool isInBuffer;

void pushFrame(CallFrame frame) {
	frames[framesCount++] = frame;

	if (framesCount >= STACK_SIZE) {
		ierr("Call frame stack overflowed.");
	}
}

CallFrame popFrame() {
	return frames[--framesCount];
}

void push(StackElem obj) {
	stack[stackCount++] = obj;

	if (stackCount >= STACK_SIZE) {
		ierr("Internal stack size exceeded.");
	}
}

StackElem pop() {
	return stack[--stackCount];
}

StackElem stackRead() {
	return stack[stackCount - 1];
}

void pushLoc() {
	locstack[locstackCount++] = instsCount;

	if (locstackCount >= STACK_SIZE) {
		ierr("Location stack size exceeded.");
	}
}

void pushSLoc(int loc) {
	locstack[locstackCount++] = loc;
}

int popLoc() {
	return locstack[--locstackCount];
}

int readLoc() {
	return locstack[locstackCount - 1];
}

void pushLoop() {
	loopstack[loopstackCount++] = instsCount;

	if (loopstackCount >= STACK_SIZE) {
		ierr("Loop stack size exceeded.");
	}
}

int popLoop() {
	return loopstack[--loopstackCount];
}

int readLoop() {
	return loopstack[loopstackCount - 1];
}

void pushArg(char* arg) {
	argstack[argstackCount++] = arg;

	if (argstackCount >= STACK_SIZE) {
		ierr("Location stack size exceeded.");
	}
}

char* popArg() {
	return argstack[--argstackCount];
}

static int pushFunc(int loc, TypeInfo type) {
	FuncPointer f;
	f.location = loc;
	f.type = type;
	f.argsLen = type.argsLen - 1;
	f.args = malloc(sizeof(char*) * f.argsLen);
	for (int i = f.argsLen - 1; i >= 0; i--) {
		f.args[i] = popArg();
	}

	funcsCount++;
	funcs = realloc(funcs, sizeof(FuncPointer) * funcsCount);
	funcs[funcsCount - 1] = f;

	return funcsCount - 1;
}

static void delVarAtIndex(NameList* names, size_t i) {
	Name name = names->names[i];
	Value var = *name.value;

	// Change ref counter
	if (isDisposable(var.type.id)) {
		var.refCount--;

		if (showDisposeInfo) {
			printf("(-) %s: %d\n", name.name, var.refCount);
		}
	}

	// Free
	free(name.name);

	// Ripple down
	for (int j = i; j < names->len - 1; j++) {
		names->names[j] = names->names[j + 1];
	}

	// Resize
	names->len--;
	names->names = (Name*) realloc(names->names, sizeof(Name) * names->len);
}

static void delVar(NameList* names, const char* v) {
	for (size_t i = 0; i < names->len; i++) {
		if (strcmp(names->names[i].name, v) == 0) {
			delVarAtIndex(v, i);
			return;
		}
	}
}

static Name* createVar(NameList* names, const char* name, Value val) {
	delVar(names, name);

	if (isDisposable(val.type.id)) {
		val.refCount = 1;
	}

	Value* ptr = malloc(sizeof(Value));
	*ptr = val;

	names->len++;
	names->names = realloc(names->names, sizeof(Name) * names->len);
	names->names[names->len - 1] = (Name){
		.name = strdup(name),
		.value = ptr,
	};

	return &names->names[names->len - 1];
}

static Name* createVarD(NameList* names, const char* name, Name* var) {
	var->value->refCount++;

	if (showDisposeInfo) {
		printf("(+) %s: %d\n", name, var->value->refCount);
	}

	delVar(names, name);

	names->len++;
	names->names = realloc(names->names, sizeof(Name) * names->len);
	names->names[names->len - 1] = (Name){
		.name = strdup(name),
		.value = var->value,
	};

	return &names->names[names->len - 1];
}

static Name* getVar(NameList* names, const char* name) {
	for (size_t i = 0; i < names->len; i++) {
		if (strcmp(names->names[i].name, name) == 0) {
			return &names->names[i];
		}
	}

	printf("Unknown variable `%s`.\n", name);
	ierr("Use of undeclared variable.");
}

static bool isVar(NameList* names, const char* name) {
	for (size_t i = 0; i < names->len; i++) {
		if (strcmp(names->names[i].name, name) == 0) {
			return true;
		}
	}

	return false;
}

void pushInst(Inst i, int scope) {
	i.scope = scope;
	i.location = instsCount;

	if (!isInBuffer) {
		instsCount++;
		insts = realloc(insts, sizeof(Inst) * instsCount);
		insts[instsCount - 1] = i;
	} else {
		curInstBuf.instsCount++;
		curInstBuf.insts = realloc(curInstBuf.insts, sizeof(Inst) * curInstBuf.instsCount);
		curInstBuf.insts[curInstBuf.instsCount - 1] = i;
	}
}

void setInst(Inst i, int loc, int scope) {
	if (isInBuffer) {
		fprintf(stderr, "Parse Error: `setInst` cannot be used in a move buffer.\n");
		exit(-1);
	}

	i.scope = scope;
	i.location = loc;

	insts[loc] = i;
}

void pushInstAt(Inst i, int loc, int scope) {
	if (isInBuffer) {
		fprintf(stderr, "Parse Error: `pushInstAt` cannot be used in a move buffer.\n");
		exit(-1);
	}

	i.scope = scope;
	i.location = loc;

	instsCount++;
	insts = realloc(insts, sizeof(Inst) * instsCount);

	for (size_t i = instsCount - 1; i > loc; i--) {
		insts[i] = insts[i - 1];
	}
	insts[loc] = i;
}

void startMoveBuffer() {
	if (isInBuffer) {
		fprintf(stderr, "Parse Error: We are already in a move buffer.\n");
		exit(-1);
	}

	instbufferCount++;
	if (instbufferCount >= STACK_SIZE) {
		fprintf(stderr, "Parse Error: Move buffer stack size exceeded.\n");
	}

	curInstBuf.insts = NULL;
	curInstBuf.instsCount = 0;
	isInBuffer = true;
}

void endMoveBuffer() {
	if (!isInBuffer) {
		fprintf(stderr, "Parse Error: We are not in a move buffer.\n");
		exit(-1);
	}

	isInBuffer = false;
}

void putMoveBuffer(int scope) {
	if (isInBuffer) {
		fprintf(stderr, "Parse Error: Cannot `putMoveBuffer` when we are in one!\n");
		exit(-1);
	}

	if (instbufferCount <= 0) {
		fprintf(stderr, "Parse Error: Move buffer stack is empty!\n");
		exit(-1);
	}

	for (size_t i = 0; i < curInstBuf.instsCount; i++) {
		pushInst(curInstBuf.insts[i], scope);
	}

	free(curInstBuf.insts);
	instbufferCount--;
}

static bool stringEqual(String a, String b) {
	if (a.len != b.len) {
		return false;
	}

	for (size_t i = 0; i < a.len; i++) {
		if (a.chars[i] != b.chars[i]) {
			return false;
		}
	}

	return true;
}

void bc_init() {
	insts = NULL;
	instsCount = 0;

	stackCount = 0;

	funcs = NULL;
	funcsCount = 0;
}

static bool instHasPointer(size_t i) {
	static_assert(_INSTS_ENUM_LEN == 41, "Update bytecode pointers.");
	switch (insts[i].inst) {
		case LOAD:
			return insts[i].type.id == TYPE_STR;
			break;
		case LOADV:
		case LOADA:
		case SAVEV:
		case ARRAYS:
		case ARRAYG:
		case THROW:
		case ACCESS:
			return true;
		default:
			return false;
	}
}

static void instDump(size_t i) {
	const char* instName;

	// clang-format off
	static_assert(_INSTS_ENUM_LEN == 41, "Update bytecode strings.");
	switch (insts[i].inst) {
		case LOAD:      instName = "load";      break;
		case LOADT: 	instName = "loadt"; 	break;
		case LOADV: 	instName = "loadv"; 	break;
		case LOADA: 	instName = "loada"; 	break;
		case SAVEV: 	instName = "savev"; 	break;
		case ASSIGNV: 	instName = "assignv"; 	break;
		case SWAPV: 	instName = "swapv"; 	break;
		case SAVEF: 	instName = "savef"; 	break;
		case CALLF: 	instName = "callf"; 	break;
		case ENDF: 		instName = "endf"; 		break;
		case STARTU: 	instName = "startu"; 	break;
		case EXTERN: 	instName = "extern"; 	break;
		case APPENDT: 	instName = "appendt"; 	break;
		case ARRAYI: 	instName = "arrayi"; 	break;
		case ARRAYIW: 	instName = "arrayiw"; 	break;
		case ARRAYIL: 	instName = "arrayil"; 	break;
		case ARRAYS: 	instName = "arrays"; 	break;
		case ARRAYG: 	instName = "arrayg"; 	break;
		case ACCESS: 	instName = "access"; 	break;
		case SWAP: 		instName = "swap"; 		break;
		case DUP: 		instName = "dup"; 		break;
		case NOT: 		instName = "not"; 		break;
		case AND: 		instName = "and"; 		break;
		case OR: 		instName = "or"; 		break;
		case ADD: 		instName = "add"; 		break;
		case SUB: 		instName = "sub"; 		break;
		case MUL: 		instName = "mul"; 		break;
		case DIV: 		instName = "div"; 		break;
		case MOD: 		instName = "mod"; 		break;
		case POW: 		instName = "pow"; 		break;
		case NEG: 		instName = "neg"; 		break;
		case EQ: 		instName = "eq"; 		break;
		case GT: 		instName = "gt"; 		break;
		case LT: 		instName = "lt"; 		break;
		case GTE: 		instName = "gte"; 		break;
		case LTE: 		instName = "lte"; 		break;
		case CAST: 		instName = "cast"; 		break;
		case GOTO: 		instName = "goto"; 		break;
		case IFN:	 	instName = "ifn"; 		break;
		case THROW:	 	instName = "throw";		break;
		default: 		instName = "?"; 		break;
	}
	// clang-format on

	if (instHasPointer(i)) {
		printf("[%ld, %d] %s: \"%s\", %s\n", i, insts[i].scope, instName,
			   (char*) insts[i].data._ptr, typestr(insts[i].type.id));
	} else {
		printf("[%ld, %d] %s: %d, %s\n", i, insts[i].scope, instName,
			   insts[i].data._int, typestr(insts[i].type.id));
	}
}

static void bc_dump() {
	for (size_t i = 0; i < instsCount; i++) {
		instDump(i);
	}
}

static void readByteCode(size_t frameIndex, size_t start, size_t endOffset) {
	int lastKnownScope = 0;
	CallFrame frame = frames[frameIndex];

	for (size_t i = start; i < instsCount - endOffset; i++) {
		if (showCount) {
			instDump(i);
		}

		// Adjust scope

		int curScope = insts[i].scope;

		if (curScope < lastKnownScope) {
			for (size_t v = 0; v < frame.names.len; v++) {
				Name name = frame.names.names[v];
				Value value = *name.value;

				if (value.scope <= curScope) {
					continue;
				}

				if (showDisposeInfo) {
					printf("Deleting : (%d > %d) `%s`\n", value.scope, curScope, name.name);
				}

				if (isDisposable(value.type.id) && value.refCount <= 1) {
					dispose(name);
				}

				delVarAtIndex(&frame.names, v);

				v--;
			}
		}
		lastKnownScope = curScope;

		static_assert(_INSTS_ENUM_LEN == 41, "Update bytecode interpreting.");
		switch (insts[i].inst) {
			case LOAD: {
				Value v = (Value){
					.type = dupTypeInfo(insts[i].type),
				};

				if (insts[i].type.id == TYPE_STR) {
					// We must convert string literals because they are c-strings
					v.data._string = cstrToStr(insts[i].data._ptr);
				} else {
					v.data = insts[i].data;
				}

				push(toElem(v));

				break;
			}
			case LOADT: {
				push(toElem((Value){.type = dupTypeInfo(insts[i].type)}));
				break;
			}
			case LOADV: {
				Name* n = getVar(&frame.names, insts[i].data._ptr);

				if (n->value->fromArgs && n->value->type.id == TYPE_FUNC) {
					ierr("Functions from arguments can only be called.");
				}

				push(toVar(n));
				break;
			}
			case LOADA: {
				pushArg(strdup(insts[i].data._ptr));
				break;
			}
			case SAVEV: {  // `a $ = b`
				StackElem b = pop();
				StackElem a = pop();

				if (isVar(&frame.names, insts[i].data._ptr)) {
					if (getVar(&frame.names, insts[i].data._ptr)->value->scope == curScope) {
						ierr("Redefinition of existing variable in the same scope.");
					}
				}

				if (a.elem.type.id != TYPE_UNKNOWN) {
					if (!typeInfoEqual(a.elem.type, getValue(b).type)) {
						ierr("Declaration type doesn't match expression.");
					}
				}

				if (b.var == NULL) {
					createVar(&frame.names, insts[i].data._ptr, b.elem);
				} else if (isDisposable(a.elem.type.id)) {
					createVarD(&frame.names, insts[i].data._ptr, b.var);
				} else {
					createVar(&frame.names, insts[i].data._ptr, *b.var->value);
				}

				break;
			}
			case ASSIGNV: {	 // `a = b`
				StackElem b = pop();
				StackElem a = pop();

				if (a.var == NULL) {
					ierr("Attempted to assign a non-variable.");
				}

				if (a.var->value->type.id == TYPE_FUNC) {
					ierr("Functions can't be reassigned at the moment.");
				}

				if (a.var->value->fromArgs) {
					ierr("Cannot assign to an argument.");
				}

				if (!typeInfoEqual(a.var->value->type, getValue(b).type)) {
					ierr("Assignment type doesn't match expression.");
				}

				if (b.var == NULL) {
					*a.var->value = b.elem;
				} else if (isDisposable(a.var->value->type.id)) {
					a.var->value = b.var->value;
				} else {
					a.var->value = dupVal(*b.var->value);
				}

				break;
			}
			case SWAPV: {  // `swap(a, b)`
				StackElem b = pop();
				StackElem a = pop();

				if (!typeInfoEqual(getValue(a).type, getValue(b).type)) {
					ierr("Attempted to swap values that don't match in type.");
				}

				if (a.var == NULL || b.var == NULL) {
					ierr("Attempted to swap values that aren't variables.");
				}

				Value* aptr = a.var->value;
				a.var->value = b.var->value;
				b.var->value = aptr;

				break;
			}
			case SAVEF: {
				StackElem a = pop();

				Value v = (Value){
					.type = dupTypeInfo(getValue(a).type),
					.data._int = pushFunc(i + 1, getValue(a).type),
				};

				push(toElem(v));
				jump(insts[i].data._int);

				break;
			}
			case CALLF:;  // `a(...)`
				a = pop();

				bool dropOutput = false;
				if (insts[i].type.id == TYPE_UNKNOWN &&
					a.type.args[0].id == TYPE_VOID) {
					ierr("Invoke expression cannot be referencing a void function.");
				} else if (insts[i].type.id == TYPE_VOID &&
						   a.type.args[0].id != TYPE_VOID) {
					dropOutput = true;
				}

				FuncPointer f = funcs[a.v.v_int];
				s = insts[f.location].scope;

				ObjectList fo;
				fo.vars = NULL;
				fo.varsCount = 0;

				if (a.fromArgs) {
					popFrame();
				}
				CallFrame funcFrame = frames[framesCount - 1];

				// Duping may not even be required?
				// TODO: Optimize
				for (size_t v = 0; v < funcFrame.o->varsCount; v++) {
					Object vobj = funcFrame.o->vars[v];
					if (vobj.scope < s) {
						dupVar(&fo, vobj);
					}
				}

				for (int v = f.argsLen - 1; v >= 0; v--) {
					Object e = pop();

					if (!typeInfoEqual(f.type.args[v + 1], e.type)) {
						printf("`%d` != `%d` in `%s`\n", e.type.id, f.type.args[v + 1].id, (char*) insts[i].a.v_ptr);
						ierr("Type mismatch in function arguments.");
					}

					Object vo = objdup(e);
					vo.name = strdup(f.args[v]);
					vo.scope = s;

					setVar(&fo, vo);
				}

				pushFrame((CallFrame){.o = &fo});
				readByteCode(framesCount - 1, f.location, 0);
				popFrame();

#define skipdelete         \
	delVarAtIndex(&fo, v); \
	v--;                   \
	continue

				// TODO: Optimize
				for (size_t v = 0; v < fo.varsCount; v++) {
					Object vobj = fo.vars[v];

					// Temporary for now
					if (vobj.type.id == TYPE_FUNC) {
						skipdelete;
					}

					// Skip if it is a local variable
					if (vobj.scope >= s) {
						skipdelete;
					}

					// Skip if the current frame doesn't contain the variable
					if (!isVar(funcFrame.o, vobj.name)) {
						skipdelete;
					}

					// Skip and delete if the variable is an argument
					if (a.fromArgs) {
						skipdelete;
					}

					// Get the var in the frame
					Object oobj = getVar(funcFrame.o, vobj.name);

					// Skip if the types are not equal
					if (!typeInfoEqual(vobj.type, oobj.type)) {
						skipdelete;
					}

					// Skip if the scopes don't match
					if (vobj.scope != oobj.scope) {
						skipdelete;
					}

					// Update the variable outside of the frame
					dupVar(funcFrame.o, vobj);
				}

#undef skipdelete

				freeObjectList(&fo);

				if (a.fromArgs) {
					pushFrame(frame);
				}

				if (dropOutput) {
					pop();
				}

				break;
			case ENDF:
				// RETURN not break
				return;
			case STARTU:;
				ObjectList* utilObjs = malloc(sizeof(ObjectList));
				utilObjs->vars = NULL;
				utilObjs->varsCount = 0;

				pushFrame((CallFrame){.o = utilObjs});
				readByteCode(framesCount - 1, i + 1, instsCount - insts[i].a.v_int);
				popFrame();

				obj = (Object){
					.referenceId = basicReference,
					.v.v_utility = (Utility){
						.o = utilObjs,
					},
					.name = strdup(popArg()),
					.scope = curScope,
					.type = type(TYPE_UTIL),
				};

				setVar(frame.o, obj);

				jump(insts[i].a.v_int);

				break;
			case EXTERN:  // `extern(..., a)`
				// Pop dummy object
				pop();

				a = pop();
				externs[a.v.v_int]();

				break;
			case APPENDT:
				b = pop();
				a = pop();

				a.type.argsLen++;
				if (a.type.args == NULL) {
					a.type.args = malloc(sizeof(TypeInfo));
					a.type.args[0] = dupTypeInfo(b.type);
				} else {
					a.type.args = realloc(a.type.args, sizeof(TypeInfo) * a.type.argsLen);
					a.type.args[a.type.argsLen - 1] = dupTypeInfo(b.type);
				}

				push(a);

				// We don't want this to get freed
				a.type = type(TYPE_VOID);

				break;
			case ARRAYI:  // `new a[b]`
				b = pop();
				a = pop();

				if (b.type.id != TYPE_INT || b.v.v_int < 0) {
					ierr("Expected positive int in array initilization.");
				}

				// Initilize the new stack element
				sobj = (Object){
					.type = type(TYPE_ARRAY),
					.referenceId = basicReference,
				};

				// Convert type into array type
				sobj.type.argsLen = 1;
				sobj.type.args = malloc(sizeof(TypeInfo));
				sobj.type.args[0] = dupTypeInfo(a.type);

				// Initilize the array
				sobj.v.v_array.arr = malloc(sizeof(Object) * b.v.v_int);
				sobj.v.v_array.len = b.v.v_int;

				// Populate array
				for (int i = 0; i < b.v.v_int; i++) {
					sobj.v.v_array.arr[i] = (Object){
						.type = dupTypeInfo(a.type),
						.v = createDefaultType(a.type),
					};

					if (isDisposable(a.type.id)) {
						size_t refId = basicReference;
						sobj.v.v_array.arr[i].referenceId = refId;
						refs[refId].counter++;
					}
				}

				// Push
				push(sobj);

				break;
			case ARRAYIW:  // `new a[b] with c`
				c = pop();
				b = pop();
				a = pop();

				if (b.type.id != TYPE_INT || b.v.v_int < 0) {
					ierr("Expected positive int in array initilization.");
				}

				if (!typeInfoEqual(a.type, c.type)) {
					ierr("The `with` expression differs in type from the array.");
				}

				// Initilize the new stack element
				sobj = (Object){
					.type = type(TYPE_ARRAY),
					.referenceId = basicReference,
				};

				// Convert type into array type
				sobj.type.argsLen = 1;
				sobj.type.args = malloc(sizeof(TypeInfo));
				sobj.type.args[0] = dupTypeInfo(a.type);

				// Initilize the array
				sobj.v.v_array.arr = malloc(sizeof(Object) * b.v.v_int);
				sobj.v.v_array.len = b.v.v_int;

				// Populate array
				for (int i = 0; i < b.v.v_int; i++) {
					sobj.v.v_array.arr[i] = c;

					if (isDisposable(a.type.id)) {
						size_t refId = basicReference;
						sobj.v.v_array.arr[i].referenceId = refId;
						refs[refId].counter++;
					}
				}

				// Push
				push(sobj);

				break;
			case ARRAYIL:;	// `new a[] { n0, n1, n2, ... }`
				Object* arrayList = malloc(sizeof(Object) * insts[i].a.v_int);
				for (int j = 0; j < insts[i].a.v_int; j++) {
					arrayList[j] = pop();
				}

				a = pop();

				// Check types
				for (int j = 0; j < insts[i].a.v_int; j++) {
					if (!typeInfoEqual(arrayList[j].type, a.type)) {
						ierr("An element in the array initialization does not match the array type.");
					}
				}

				// Initilize the new stack element
				sobj = (Object){
					.type = type(TYPE_ARRAY),
					.referenceId = basicReference,
				};

				// Convert type into array type
				sobj.type.argsLen = 1;
				sobj.type.args = malloc(sizeof(TypeInfo));
				sobj.type.args[0] = dupTypeInfo(a.type);

				// Convert the object list to a ValueHolder list
				sobj.v.v_array.arr = malloc(sizeof(Object) * insts[i].a.v_int);
				sobj.v.v_array.len = insts[i].a.v_int;

				// Populate
				for (int j = 0; j < insts[i].a.v_int; j++) {
					int arrIndex = insts[i].a.v_int - j - 1;
					sobj.v.v_array.arr[j] = arrayList[arrIndex];
					refs[arrayList[arrIndex].referenceId].counter++;
				}

				// Push
				push(sobj);

				// Free
				free(arrayList);

				break;
			case ARRAYS:  // `c[a] = b`
				b = pop();
				a = pop();
				c = pop();

				if (a.type.id != TYPE_INT) {
					ierr("Index must be an int.");
				}

				if (a.v.v_int < 0) {
					ierr("Index must be more than 0.");
				}

				if (c.type.id != TYPE_ARRAY) {
					ierr("The object referenced is not an array.");
				}

				arr = c.v.v_array;

				if (a.v.v_int >= arr.len) {
					ierr("Index must be less than the length.");
				}

				if (!typeInfoEqual(b.type, c.type.args[0])) {
					ierr("Set type doens't match expression.");
				}

				arr.arr[a.v.v_int] = b;

				break;
			case ARRAYG:  // `a[b]`
				b = pop();
				a = pop();

				if (b.type.id != TYPE_INT) {
					ierr("Index must be an int.");
				}

				if (b.v.v_int < 0) {
					ierr("Index must be more than 0.");
				}

				if (a.type.id != TYPE_ARRAY) {
					ierr("The object referenced is not an array.");
				}

				arr = a.v.v_array;

				if (b.v.v_int >= arr.len) {
					ierr("Index must be less than the length.");
				}

				push(arr.arr[b.v.v_int]);

				break;
			case ACCESS:  // `a.$`
				a = pop();

				if (a.type.id == TYPE_ARRAY) {
					if (strcmp(insts[i].a.v_ptr, "length") != 0) {
						ierr("The only accessible member of an array is `length`.");
					}

					arr = a.v.v_array;

					push((Object){
						.type = type(TYPE_INT),
						.v.v_int = arr.len,
					});
				} else if (a.type.id == TYPE_UTIL) {
					sobj = getVar(a.v.v_utility.o, insts[i].a.v_ptr);
					sobj.type = dupTypeInfo(sobj.type);

					push(sobj);
				} else {
					ierr("The object referenced does not have accessible members.");
				}

				break;
			case SWAP:
				b = stack[stackCount - 1];
				a = stack[stackCount - 2];

				stack[stackCount - 2] = b;
				stack[stackCount - 1] = a;

				// We don't want this to get freed
				a = (Object){0};
				b = (Object){0};

				break;
			case DUP:
				obj = stackRead();
				obj.type = dupTypeInfo(obj.type);
				push(obj);

				break;
			case NOT:
				a = pop();

				if (a.type.id == TYPE_BOOL) {
					push((Object){.type = type(TYPE_BOOL), .v.v_int = !a.v.v_int});
				} else {
					ierr("Invalid type for `not`.");
				}

				break;
			case AND:
				b = pop();
				a = pop();

				if (a.type.id == TYPE_BOOL && b.type.id == TYPE_BOOL) {
					push((Object){.type = type(TYPE_BOOL), .v.v_int = a.v.v_int && b.v.v_int});
				} else {
					ierr("Invalid types for `and`.");
				}

				break;
			case OR:
				b = pop();
				a = pop();

				if (a.type.id == TYPE_BOOL && b.type.id == TYPE_BOOL) {
					push((Object){.type = type(TYPE_BOOL), .v.v_int = a.v.v_int || b.v.v_int});
				} else {
					ierr("Invalid types for `and`.");
				}

				break;
			case ADD:
				b = pop();
				a = pop();

				if (a.type.id == TYPE_INT && b.type.id == TYPE_INT) {
					push((Object){.type = type(TYPE_INT), .v.v_int = a.v.v_int + b.v.v_int});
				} else if (a.type.id == TYPE_FLOAT && b.type.id == TYPE_FLOAT) {
					push((Object){.type = type(TYPE_FLOAT), .v.v_float = a.v.v_float + b.v.v_float});
				} else if (a.type.id == TYPE_LONG && b.type.id == TYPE_LONG) {
					push((Object){.type = type(TYPE_LONG), .v.v_long = a.v.v_long + b.v.v_long});
				} else if (a.type.id == TYPE_DOUBLE && b.type.id == TYPE_DOUBLE) {
					push((Object){.type = type(TYPE_DOUBLE), .v.v_double = a.v.v_double + b.v.v_double});
				} else if (a.type.id == TYPE_STR && b.type.id == TYPE_STR) {
					// Create string
					String s = (String){
						.len = a.v.v_string.len + b.v.v_string.len,
					};
					s.chars = malloc(s.len);

					// Combine the two strings
					memcpy(s.chars, a.v.v_string.chars, a.v.v_string.len);
					memcpy(s.chars + a.v.v_string.len, b.v.v_string.chars, b.v.v_string.len);

					// Push onto stack
					push((Object){
						.type = type(TYPE_STR),
						.v.v_string = s,
						.referenceId = basicReference,
					});
				} else {
					ierr("Invalid types for `add`.");
				}

				break;
			case SUB:
				basicOperation(-);
				break;
			case MUL:
				basicOperation(*);
				break;
			case DIV:
				basicOperation(/);
				break;
			case MOD:
				b = pop();
				a = pop();
				if (a.type.id == TYPE_INT && b.type.id == TYPE_INT) {
					int answer = a.v.v_int % b.v.v_int;
					if (answer < 0) {
						answer += b.v.v_int;
					}

					push((Object){.type = type(TYPE_INT), .v.v_int = answer});
				} else if (a.type.id == TYPE_LONG && b.type.id == TYPE_LONG) {
					long answer = a.v.v_long % b.v.v_long;
					if (answer < 0) {
						answer += b.v.v_long;
					}

					push((Object){.type = type(TYPE_LONG), .v.v_long = answer});
				} else {
					ierr("Invalid types for `%%`.");
				}

				break;
			case POW:  // `a ^ b`
				b = pop();
				a = pop();
				if (a.type.id == TYPE_INT && b.type.id == TYPE_INT) {
					int number = 1;

					if (b.v.v_int < 0) {
						number = 0;
					} else {
						for (int j = 0; j < b.v.v_int; j++) {
							number *= a.v.v_int;
						}
					}

					push((Object){.type = type(TYPE_INT), .v.v_int = number});
				} else if (a.type.id == TYPE_LONG && b.type.id == TYPE_LONG) {
					long number = 1;

					if (b.v.v_long < 0) {
						number = 0;
					} else {
						for (long j = 0; j < b.v.v_long; j++) {
							number *= a.v.v_long;
						}
					}

					push((Object){.type = type(TYPE_LONG), .v.v_long = number});
				} else if (a.type.id == TYPE_FLOAT && b.type.id == TYPE_FLOAT) {
					push((Object){.type = type(TYPE_FLOAT), .v.v_float = powf(a.v.v_float, b.v.v_float)});
				} else if (a.type.id == TYPE_DOUBLE && b.type.id == TYPE_DOUBLE) {
					push((Object){.type = type(TYPE_DOUBLE), .v.v_double = pow(a.v.v_double, b.v.v_double)});
				} else {
					ierr("Invalid types for `^`.");
				}

				break;
			case NEG:
				a = pop();
				if (a.type.id == TYPE_INT) {
					push((Object){.type = type(TYPE_INT), .v.v_int = -a.v.v_int});
				} else if (a.type.id == TYPE_FLOAT) {
					push((Object){.type = type(TYPE_FLOAT), .v.v_float = -a.v.v_float});
				} else if (a.type.id == TYPE_DOUBLE) {
					push((Object){.type = type(TYPE_DOUBLE), .v.v_double = -a.v.v_double});
				} else if (a.type.id == TYPE_LONG) {
					push((Object){.type = type(TYPE_LONG), .v.v_long = -a.v.v_long});
				} else {
					ierr("Invalid types for `neg`");
				}

				break;
			case EQ:
				b = pop();
				a = pop();

				if (a.type.id == TYPE_INT && b.type.id == TYPE_INT ||
					a.type.id == TYPE_FUNC && b.type.id == TYPE_FUNC ||
					a.type.id == TYPE_BOOL && b.type.id == TYPE_BOOL) {
					push((Object){.type = type(TYPE_BOOL), .v.v_int = a.v.v_int == b.v.v_int});
				} else if (a.type.id == TYPE_LONG && b.type.id == TYPE_LONG) {
					push((Object){.type = type(TYPE_BOOL), .v.v_int = a.v.v_long == b.v.v_long});
				} else if (a.type.id == TYPE_STR && a.type.id == TYPE_STR) {
					push((Object){
						.type = type(TYPE_BOOL),
						.v.v_int = stringEqual(a.v.v_string, b.v.v_string),
					});
				} else {
					ierr("Invalid types for `eq`.");
				}

				break;
			case GT:
				boolOperation(>);
				break;
			case LT:
				boolOperation(<);
				break;
			case GTE:
				boolOperation(>=);
				break;
			case LTE:
				boolOperation(<=);
				break;
			case CAST:
				a = pop();

				if (insts[i].type.id == TYPE_STR) {
					if (a.type.id == TYPE_INT) {
						toStr(a.v.v_int, "%d");
					} else if (a.type.id == TYPE_LONG) {
						toStr(a.v.v_long, "%ld");
					} else if (a.type.id == TYPE_FLOAT) {
						toStr(a.v.v_float, "%f");
					} else if (a.type.id == TYPE_DOUBLE) {
						toStr(a.v.v_double, "%lf");
					} else if (a.type.id == TYPE_BOOL) {
						if (a.v.v_int) {
							push((Object){
								.type = type(TYPE_STR),
								.v.v_string = cstrToStr("true"),
								.referenceId = basicReference,
							});
						} else {
							push((Object){
								.type = type(TYPE_STR),
								.v.v_string = cstrToStr("false"),
								.referenceId = basicReference,
							});
						}
					} else {
						printf("Cannot cast type %d.\n", a.type.id);
						ierr("Invalid type for `cast` to `string`.");
					}
				} else if (insts[i].type.id == TYPE_INT) {
					if (a.type.id == TYPE_LONG) {
						push((Object){
							.type = type(TYPE_INT),
							.v.v_int = (int) a.v.v_long,
						});
					} else {
						printf("Cannot cast type %d.\n", a.type.id);
						ierr("Invalid type for `cast` to `int`.");
					}
				} else if (insts[i].type.id == TYPE_LONG) {
					if (a.type.id == TYPE_INT) {
						push((Object){
							.type = type(TYPE_LONG),
							.v.v_long = (long) a.v.v_int,
						});
					} else {
						printf("Cannot cast type %d.\n", a.type.id);
						ierr("Invalid type for `cast` to `long`.");
					}
				} else if (insts[i].type.id == TYPE_FLOAT) {
					if (a.type.id == TYPE_DOUBLE) {
						push((Object){
							.type = type(TYPE_FLOAT),
							.v.v_float = (float) a.v.v_double,
						});
					} else if (a.type.id == TYPE_INT) {
						push((Object){
							.type = type(TYPE_FLOAT),
							.v.v_float = (float) a.v.v_int,
						});
					} else if (a.type.id == TYPE_LONG) {
						push((Object){
							.type = type(TYPE_FLOAT),
							.v.v_float = (float) a.v.v_long,
						});
					} else {
						printf("Cannot cast type %d.\n", a.type.id);
						ierr("Invalid type for `cast` to `float`.");
					}
				} else if (insts[i].type.id == TYPE_DOUBLE) {
					if (a.type.id == TYPE_FLOAT) {
						push((Object){
							.type = type(TYPE_DOUBLE),
							.v.v_double = (double) a.v.v_float,
						});
					} else if (a.type.id == TYPE_INT) {
						push((Object){
							.type = type(TYPE_DOUBLE),
							.v.v_double = (double) a.v.v_int,
						});
					} else if (a.type.id == TYPE_LONG) {
						push((Object){
							.type = type(TYPE_DOUBLE),
							.v.v_double = (double) a.v.v_long,
						});
					} else {
						printf("Cannot cast type %d.\n", a.type.id);
						ierr("Invalid type for `cast` to `double`.");
					}
				}

				break;
			case GOTO:
				jump(insts[i].a.v_int);
				break;
			case IFN:
				a = pop();

				if (a.type.id != TYPE_BOOL) {
					ierr("Invalid type for `ifn`.");
				}

				if (a.v.v_int == 0) {
					jump(insts[i].a.v_int);
				}

				break;
			case THROW:
				fprintf(stderr, "%s\n", (char*) insts[i].a.v_ptr);
				ierr("An error has been thrown, and execution has been aborted.");

				return;
			default:
				break;
		}

		if (a.type.args != NULL) {
			freeTypeInfo(a.type);
		}

		if (b.type.args != NULL) {
			freeTypeInfo(b.type);
		}

		if (c.type.args != NULL) {
			freeTypeInfo(c.type);
		}
	}
}

void bc_run(bool showByteCode) {
	if (showByteCode) {
		bc_dump();
		return;
	}

	// Read byte code

	ObjectList o;
	o.vars = NULL;
	o.varsCount = 0;
	pushFrame((CallFrame){.o = &o});

	readByteCode(framesCount - 1, 0, 0);

	popFrame();
	freeObjectList(&o);
}

void bc_end() {
	// Check for stack leaks

	if (stackCount != 0) {
		fprintf(stderr, "Stack Error: Stack leak detected. Ending size `%ld`.\n", stackCount);
	}

	if (locstackCount != 0) {
		fprintf(stderr, "Stack Error: Location stack leak detected. Ending size `%ld`.\n", locstackCount);
	}

	if (framesCount != 0) {
		fprintf(stderr, "Stack Error: CallFrame stack leak detected. Ending size `%ld`.\n", framesCount);
	}

	// Check for reference leaks

	for (size_t i = 0; i < refsCount; i++) {
		if (refs[i].counter > 0) {
			// fprintf(stderr, "Reference Warning: Reference `%ld` was not freed. It will be leaked.\n", i);
		}
	}

	// Free instructions

	for (size_t i = 0; i < instsCount; i++) {
		if (insts[i].type.args != NULL) {
			freeTypeInfo(insts[i].type);
		}

		// Double frees
		/*if (instHasPointer(i)) {
			free(insts[i].a.v_ptr);
		}*/
	}

	free(insts);

	// End

	free(funcs);
	free(refs);
	// malloc_stats();
}