#include "bytecode.h"

// TODO: Proper accessors `.`
// TODO: Redo `repeat`
// TODO: Utilities
// TODO: Types
// TODO: size_t for goto and stuff; or relative addresses
// TODO: static checking

#define singleOperation(opName)                                           \
	Value a = getValue(pop());                                            \
	SingleOperation op = types[a.type.id].opName;                         \
	if (op != NULL) {                                                     \
		push(toElem(op(a)));                                              \
	} else {                                                              \
		ierr("The object used does not have a `" #opName "` operation."); \
	}

#define doubleOperation(opName)                                                   \
	Value b = getValue(pop());                                                    \
	Value a = getValue(pop());                                                    \
	if (!typeInfoEqual(a.type, b.type)) {                                         \
		fprintf(stderr, "`%s` != `%s`\n", typestr(a.type), typestr(b.type));      \
		ierr("The types of the two objects used in `" #opName "` do not match."); \
	}                                                                             \
	DoubleOperation op = types[a.type.id].opName;                                 \
	if (op != NULL) {                                                             \
		push(toElem(op(a, b)));                                                   \
	} else {                                                                      \
		ierr("The object used does not have a `" #opName "` operation.");         \
	}

#define ierr(...)                                          \
	fprintf(stderr, "Interpret Error: " __VA_ARGS__ "\n"); \
	exit(-1);

#define iwarn(...) fprintf(stdout, "Interpret Warning: " __VA_ARGS__ "\n");

#define jump(loc) i = loc - 1;

#define curInstBuf instbuffer[instbufferCount - 1]

typedef struct {
	NameList* names;
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

		if (var.refCount <= 0) {
			free(name.value);
		}
	} else {
		// free(name.value);
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
			delVarAtIndex(names, i);
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

static Name* setVar(NameList* names, const char* name, Name* var) {
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

	fprintf(stderr, "Unknown variable `%s`.\n", name);
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

void bc_init() {
	insts = NULL;
	instsCount = 0;

	stackCount = 0;

	funcs = NULL;
	funcsCount = 0;
}

static bool instHasPointer(size_t i) {
	static_assert(_INSTS_ENUM_LEN == 40, "Update bytecode pointers.");
	switch (insts[i].inst) {
		case LOAD:
			return insts[i].type.id == TYPE_STR;
			break;
		case LOADV:
		case LOADA:
		case SAVEV:
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
	static_assert(_INSTS_ENUM_LEN == 40, "Update bytecode strings.");
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
			   (char*) insts[i].data._ptr, typestr(insts[i].type));
	} else {
		printf("[%ld, %d] %s: %d, %s\n", i, insts[i].scope, instName,
			   insts[i].data._int, typestr(insts[i].type));
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
			for (size_t v = 0; v < frame.names->len; v++) {
				Name name = frame.names->names[v];
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

				delVarAtIndex(frame.names, v);

				v--;
			}
		}
		lastKnownScope = curScope;

		static_assert(_INSTS_ENUM_LEN == 40, "Update bytecode interpreting.");
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
				Name* n = getVar(frame.names, insts[i].data._ptr);

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
				Value bval = getValue(b);
				StackElem a = pop();

				if (isVar(frame.names, insts[i].data._ptr)) {
					if (getVar(frame.names, insts[i].data._ptr)->value->scope == curScope) {
						ierr("Redefinition of existing variable in the same scope.");
					}
				}

				if (a.elem.type.id != TYPE_UNKNOWN) {
					if (!typeInfoEqual(a.elem.type, bval.type)) {
						fprintf(stderr, "`%s` != `%s`\n", typestr(a.elem.type),
								typestr(bval.type));
						ierr("Declaration type doesn't match expression.");
					}
				}

				if (b.var == NULL) {
					createVar(frame.names, insts[i].data._ptr, b.elem);
				} else if (isDisposable(a.elem.type.id)) {
					setVar(frame.names, insts[i].data._ptr, b.var);
				} else {
					createVar(frame.names, insts[i].data._ptr, *b.var->value);
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
					Value* p = malloc(sizeof(Value));
					*p = dupValue(*b.var->value);
					a.var->value = p;
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
				Value a = getValue(pop());

				Value v = (Value){
					.type = dupTypeInfo(a.type),
					.data._int = pushFunc(i + 1, a.type),
				};

				push(toElem(v));
				jump(insts[i].data._int);

				break;
			}
			case CALLF: {  // `a(...)`
				StackElem a = pop();
				Value av = getValue(a);

				bool dropOutput = false;
				if (insts[i].type.id == TYPE_UNKNOWN &&
					av.type.args[0].id == TYPE_VOID) {
					ierr("Invoke expression cannot be referencing a void function.");
				} else if (insts[i].type.id == TYPE_VOID &&
						   av.type.args[0].id != TYPE_VOID) {
					dropOutput = true;
				}

				FuncPointer f = funcs[av.data._int];
				int s = insts[f.location].scope;

				NameList names;
				names.names = NULL;
				names.len = 0;

				if (av.fromArgs) {
					popFrame();
				}
				CallFrame funcFrame = frames[framesCount - 1];

				// TODO: Optimize
				for (size_t v = 0; v < funcFrame.names->len; v++) {
					Name name = funcFrame.names->names[v];
					if (name.value->scope < s) {
						setVar(&names, name.name, &name);
					}
				}

				for (int j = f.argsLen - 1; j >= 0; j--) {
					StackElem arg = pop();
					Value argv = getValue(arg);

					if (!typeInfoEqual(f.type.args[j + 1], argv.type)) {
						printf("`%s` != `%s` in `%s`\n", typestr(argv.type),
							   typestr(f.type.args[j + 1]), (char*) insts[i].data._ptr);
						ierr("Type mismatch in function arguments.");
					}

					createVar(&names, strdup(f.args[j]), dupValue(argv));
				}

				pushFrame((CallFrame){.names = &names});
				readByteCode(framesCount - 1, f.location, 0);
				popFrame();

				// free(names.names);

				if (av.fromArgs) {
					pushFrame(frame);
				}

				if (dropOutput) {
					pop();
				}

				break;
			}
			case ENDF:
				// RETURN not break
				return;
			case STARTU: {
				pushFrame((CallFrame){0});
				readByteCode(framesCount - 1, i + 1, instsCount - insts[i].data._int);
				NameList* members = popFrame().names;

				Value v = (Value){
					.type = type(TYPE_UTIL),
					.scope = curScope,
				};

				v.data._utility = (Utility){
					.members = members,
				},

				createVar(frame.names, strdup(popArg()), v);

				jump(insts[i].data._int);

				break;
			}
			case EXTERN: {	// `extern(..., a)`
				// Pop dummy object
				pop();

				StackElem a = pop();
				int index = getValue(a).data._int;

				if (index < 0 || index >= externLen) {
					ierr("Unknown extern index.");
				}

				externs[index]();

				break;
			}
			case APPENDT: {
				Value b = pop().elem;
				Value a = pop().elem;

				a.type.argsLen++;
				if (a.type.args == NULL) {
					a.type.args = malloc(sizeof(TypeInfo));
					a.type.args[0] = dupTypeInfo(b.type);
				} else {
					a.type.args = realloc(a.type.args, sizeof(TypeInfo) * a.type.argsLen);
					a.type.args[a.type.argsLen - 1] = dupTypeInfo(b.type);
				}

				push(toElem(a));

				break;
			}
			case ARRAYI: {	// `new a[b]`
				Value b = getValue(pop());
				Value a = pop().elem;

				if (b.type.id != TYPE_INT || b.data._int < 0) {
					ierr("Expected positive int in array initilization.");
				}

				// Initilize the new stack element
				Value outv = (Value){
					.type = type(TYPE_ARRAY),
				};

				// Convert type into array type
				outv.type.argsLen = 1;
				outv.type.args = malloc(sizeof(TypeInfo));
				outv.type.args[0] = dupTypeInfo(a.type);

				// Initilize the array
				outv.data._array.arr = malloc(sizeof(Value*) * b.data._int);
				outv.data._array.len = b.data._int;

				// Populate array
				for (int i = 0; i < b.data._int; i++) {
					outv.data._array.arr[i] = malloc(sizeof(Value));
					*outv.data._array.arr[i] = (Value){
						.type = dupTypeInfo(a.type),
						.data = createDefaultType(a.type),
					};

					if (isDisposable(a.type.id)) {
						outv.data._array.arr[i]->refCount++;
					}
				}

				// Push
				push(toElem(outv));

				break;
			}
			case ARRAYIW: {	 // `new a[b] with c`
				StackElem c = pop();
				Value b = getValue(pop());
				Value a = pop().elem;

				if (b.type.id != TYPE_INT || b.data._int < 0) {
					ierr("Expected positive int in array initilization.");
				}

				if (!typeInfoEqual(a.type, getValue(c).type)) {
					ierr("The `with` expression differs in type from the array.");
				}

				// Initilize the new array
				Value outv = (Value){
					.type = type(TYPE_ARRAY),
				};

				// Convert type into array type
				outv.type.argsLen = 1;
				outv.type.args = malloc(sizeof(TypeInfo));
				outv.type.args[0] = dupTypeInfo(a.type);

				// Initilize the array
				outv.data._array.arr = malloc(sizeof(Value*) * b.data._int);
				outv.data._array.len = b.data._int;

				// Populate array
				if (isDisposable(a.type.id)) {
					for (int i = 0; i < b.data._int; i++) {
						Value* v = getValuePtr(c);
						outv.data._array.arr[i] = v;
						v->refCount++;
					}
				} else {
					for (int i = 0; i < b.data._int; i++) {
						Value* p = malloc(sizeof(Value));
						*p = dupValue(getValue(c));
						outv.data._array.arr[i] = p;
					}
				}

				// Push
				push(toElem(outv));

				break;
			}
			case ARRAYIL: {	 // `new a[] { n0, n1, n2, ... }`
				StackElem* arrElements = malloc(sizeof(StackElem) * insts[i].data._int);
				for (int j = 0; j < insts[i].data._int; j++) {
					arrElements[j] = pop();
				}

				Value a = pop().elem;

				// Check types
				for (int j = 0; j < insts[i].data._int; j++) {
					if (!typeInfoEqual(getValue(arrElements[j]).type, a.type)) {
						ierr("An element in the array initialization does not match the array type.");
					}
				}

				// Initilize the new stack element
				Value outv = (Value){
					.type = type(TYPE_ARRAY),
				};

				// Convert type into array type
				outv.type.argsLen = 1;
				outv.type.args = malloc(sizeof(TypeInfo));
				outv.type.args[0] = dupTypeInfo(a.type);

				// Convert the object list to a ValueHolder list
				outv.data._array.arr = malloc(sizeof(Value*) * insts[i].data._int);
				outv.data._array.len = insts[i].data._int;

				// Populate
				if (isDisposable(a.type.id)) {
					for (int j = 0; j < insts[i].data._int; j++) {
						int arrIndex = insts[i].data._int - j - 1;

						Value* v = getValuePtr(arrElements[arrIndex]);
						outv.data._array.arr[j] = v;
						v->refCount++;
					}
				} else {
					for (int j = 0; j < insts[i].data._int; j++) {
						int arrIndex = insts[i].data._int - j - 1;

						Value* p = malloc(sizeof(Value));
						*p = dupValue(getValue(arrElements[arrIndex]));
						outv.data._array.arr[j] = p;
					}
				}

				// Push
				push(toElem(outv));

				// Free
				free(arrElements);

				break;
			}
			case ARRAYG: {	// `a[b]`
				Value b = getValue(pop());
				Value a = getValue(pop());

				if (b.type.id != TYPE_INT) {
					ierr("Index must be an int.");
				}

				if (b.data._int < 0) {
					ierr("Index must be more than 0.");
				}

				if (a.type.id != TYPE_ARRAY) {
					ierr("The object referenced is not an array.");
				}

				if (b.data._int >= a.data._array.len) {
					ierr("Index must be less than the length.");
				}

				Name* name = malloc(sizeof(Name));
				*name = (Name){
					.name = NULL,
					.value = a.data._array.arr[b.data._int],
				};
				push(toVar(name));

				break;
			}
			case ACCESS: {	// `a.$`
				Value a = getValue(pop());

				if (a.type.id == TYPE_ARRAY) {
					if (strcmp(insts[i].data._ptr, "length") != 0) {
						ierr("The only accessible member of an array is `length`.");
					}

					Value v = (Value){
						.type = type(TYPE_INT),
					};
					v.data._int = a.data._array.len;

					push(toElem(v));
				} else if (a.type.id == TYPE_UTIL) {
					push(toVar(getVar(a.data._utility.members, insts[i].data._ptr)));
				} else {
					ierr("The object referenced does not have accessible members.");
				}

				break;
			}
			case SWAP: {
				StackElem b = stack[stackCount - 1];
				StackElem a = stack[stackCount - 2];

				stack[stackCount - 2] = b;
				stack[stackCount - 1] = a;

				break;
			}
			case DUP: {
				StackElem elem = stackRead();
				// elem.type = dupTypeInfo(elem.type);
				push(elem);

				break;
			}
			case NOT: {	 // !a
				singleOperation(opNot);
				break;
			}
			case AND: {	 // a && b
				doubleOperation(opAnd);
				break;
			}
			case OR: {	// a || b
				doubleOperation(opOr);
				break;
			}
			case ADD: {	 // a + b
				doubleOperation(opAdd);
				break;
			}
			case SUB: {	 // a - b
				doubleOperation(opSub);
				break;
			}
			case MUL: {	 // a * b
				doubleOperation(opMul);
				break;
			}
			case DIV: {	 // a / b
				doubleOperation(opDiv);
				break;
			}
			case MOD: {	 // a % b
				doubleOperation(opMod);
				break;
			}
			case POW: {	 // a ^ b
				doubleOperation(opPow);
				break;
			}
			case NEG: {	 // -a
				singleOperation(opNeg);
				break;
			}
			case EQ: {	// a == b
				doubleOperation(opEq);
				break;
			}
			case GT: {	// a > b
				doubleOperation(opGt);
				break;
			}
			case LT: {	// a < b
				doubleOperation(opLt);
				break;
			}
			case GTE: {	 // a >= b
				doubleOperation(opGte);
				break;
			}
			case LTE: {	 // a <= b
				doubleOperation(opLte);
				break;
			}
			case CAST: {  // (a) b
				Value b = getValue(pop());
				Value a = pop().elem;

				CastOperation op = types[b.type.id].castTo;
				if (op != NULL) {
					push(toElem(op(a.type, b)));
				} else {
					ierr("Specified type is not castable.");
				}

				break;
			}
			case GOTO: {
				jump(insts[i].data._int);
				break;
			}
			case IFN: {
				Value a = getValue(pop());

				if (a.type.id != TYPE_BOOL) {
					ierr("Invalid type for `ifn`.");
				}

				if (a.data._int == 0) {
					jump(insts[i].data._int);
				}

				break;
			}
			case THROW: {
				fprintf(stderr, "%s\n", (char*) insts[i].data._ptr);
				ierr("An error has been thrown, and execution has been aborted.");

				return;
			}
			default:
				break;
		}
	}
}

void bc_run(bool showByteCode) {
	if (showByteCode) {
		bc_dump();
		return;
	}

	// Read byte code

	NameList names = (NameList){0};
	pushFrame((CallFrame){.names = &names});

	readByteCode(framesCount - 1, 0, 0);

	popFrame();
	free(names.names);
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
	// malloc_stats();
}