%code requires {
	#include "../bytecode.h"
	#include "../flags.h"
}

%{
	#include <stdlib.h>
	#include <stdio.h>
	#include <string.h>
	
	FILE* yyin;
	int scope = 0;
	int internalVarId = 0;
	
	#define pushi(...) pushInst((Inst) __VA_ARGS__, scope);
	
	#define repVarName strdup((char*) stackRead().v.v_ptr)

	int yywrap() {
		return 1;
	}
	
	int yyerror(const char* s) {
		fprintf(stderr, "Parse Error: %s \n", s);
		exit(-1);
	}
%}

/* Settings */
%define parse.error verbose

/* Union declaration */
%union {
	char* v_str;
	int v_int;
	float v_float;
	long v_long;
	double v_double;
}

/* Important */
%token EOL
%token<v_str> IDENTIFIER

/* Statement keywords */
%token S_EXTERN S_IF S_ELSE S_WHILE S_RETURN S_BREAK S_FUNC S_SWAP S_FOR S_THROW
%token S_REPEAT S_UTILITY S_OBJECT

/* Expression keywords */
%token E_NEW E_WITH E_COPY

/* Built-in types & literals */
%token T_AUTO T_VOID T_STR T_INT T_BOOL T_FLOAT T_FUNC T_LONG T_DOUBLE
%token<v_int> L_NUMBER L_BOOL
%token<v_str> L_STRING 
%token<v_float> L_FLOAT
%token<v_long> L_LONG
%token<v_double> L_DOUBLE

/* Operators */
%token O_EQ O_NE O_AND O_OR O_GTE O_LTE O_EB
%left O_AND O_OR
%left '>' '<' O_EQ O_NE O_GTE O_LTE
%left '+' '-'
%left '*' '/' '%'
%left O_CAST
%left '!' O_UMINUS
%left '^'
%left O_ARR_GET
%left O_ARR_INIT
%left '.'

%%

/* Important */

program 	: /* Nothing */
			| program statement EOL
			| program estatement
			| program block
			| program EOL
			;

block		:	{
					scope++;
				} s_block {
					scope--;
				}
			;

s_block		: '{' program '}'
			;

/* Types */

type		: T_VOID {
					pushi({.inst = LOADT, .type = type(TYPE_VOID)});
				}
			| T_STR {
					pushi({.inst = LOADT, .type = type(TYPE_STR)});
				}
			| T_INT {
					pushi({.inst = LOADT, .type = type(TYPE_INT)});
				}
			| T_BOOL {
					pushi({.inst = LOADT, .type = type(TYPE_BOOL)});
				}
			| T_FLOAT {
					pushi({.inst = LOADT, .type = type(TYPE_FLOAT)});
				}
			| T_LONG {
					pushi({.inst = LOADT, .type = type(TYPE_LONG)});
				}
			| T_DOUBLE {
					pushi({.inst = LOADT, .type = type(TYPE_DOUBLE)});
				}
			| T_FUNC {
					pushi({.inst = LOADT, .type = type(TYPE_FUNC)});
				} '<' type_list '>'
			| expr {
					pushi({.inst = LOADOT});
				}
			| type O_EB {
					pushi({.inst = LOADT, .type = type(TYPE_ARRAY)});
					pushi({.inst = SWAP});
					pushi({.inst = APPENDT});
				}
			;

type_list	: type {
					pushi({.inst = APPENDT});
				} tl_elem
			;

tl_elem		: /* Nothing */
			| ',' type {
					pushi({.inst = APPENDT});
				} tl_elem
			;

/* Statement and expression lists */

estatement	: if
			| while
			| declaref
			| for
			| repeat
			| utility
			| object
			;

statement	: declare
			| assign
			| invoke_s
			| return
			| break
			| extern
			| swap
			| inc_dec
			| throw
			| op_assign
			| fwd_object
			;

expr		: '(' expr ')'
			| L_NUMBER { pushi({.inst = LOAD, .type = type(TYPE_INT), .data._int = $1}); }
			| L_STRING { pushi({.inst = LOAD, .type = type(TYPE_STR), .data._ptr = $1}); }
			| L_BOOL { pushi({.inst = LOAD, .type = type(TYPE_BOOL), .data._int = $1}); }
			| L_FLOAT { pushi({.inst = LOAD, .type = type(TYPE_FLOAT), .data._float = $1}); }
			| L_LONG { pushi({.inst = LOAD, .type = type(TYPE_LONG), .data._long = $1}); }
			| L_DOUBLE { pushi({.inst = LOAD, .type = type(TYPE_DOUBLE), .data._double = $1}); }
			| IDENTIFIER { pushi({.inst = LOADV, .data._ptr = $1}); }
			| num_op
			| bool_op
			| cast
			| access
			| elambda /* Explict Lambda */
			| invoke_e
			| extern
			| arr_init
			| arr_get
			| init_object
			| copy
			;

/* Basic Statements */

declare		: type IDENTIFIER '=' expr { 
					pushi({.inst = SAVEV, .data._ptr = $2});
				}
			| T_AUTO IDENTIFIER {
					pushi({.inst = LOADT, .type = type(TYPE_UNKNOWN)});
				} '=' expr {
					pushi({.inst = SAVEV, .data._ptr = $2});
				}
			;

assign		: expr '=' expr { 
					pushi({.inst = ASSIGNV}); 
				}
			;

op_assign	: expr '+' '=' {
					pushi({.inst = DUP});
				} expr {
					pushi({.inst = ADD});
					pushi({.inst = ASSIGNV});
				}
			| expr '-' '=' {
					pushi({.inst = DUP});
				} expr {
					pushi({.inst = SUB});
					pushi({.inst = ASSIGNV});
				}
			| expr '*' '=' {
					pushi({.inst = DUP});
				} expr {
					pushi({.inst = MUL});
					pushi({.inst = ASSIGNV});
				}
			| expr '/' '=' {
					pushi({.inst = DUP});
				} expr {
					pushi({.inst = DIV});
					pushi({.inst = ASSIGNV});
				}
			;

while		: S_WHILE {
					pushLoc();
				} '(' expr ')' {
					pushLoc();
					pushLoop();
					pushi({});
				} block {
					popLoop();
					setInst((Inst){.inst = IFN, .data._int = instsCount + 1}, popLoc(), scope);
					pushi({.inst = GOTO, .data._int = popLoc()});
				}
			;

for			: S_FOR '(' {
					scope++;
				} declare ',' {
					pushLoc();
					scope++;
				} expr ',' {
					pushLoc();
					pushLoop();
					pushi({});
					startMoveBuffer();
				} statement {
					endMoveBuffer();
				} ')' s_block {
					putMoveBuffer(scope);
					popLoop();
					setInst((Inst){.inst = IFN, .data._int = instsCount + 1}, popLoc(), scope);
					scope--;
					pushi({.inst = GOTO, .data._int = popLoc()});
					scope--;
					
					// Push padding to dispose variables
					pushi({.inst = -1});
				}
			;

repeat		: S_REPEAT '(' expr ')' s_block {
					yyerror("REPEAT will be disabled until preprocessing is added.");
				}
			;

break		: S_BREAK {
					pushi({.inst = LOAD, .type = type(TYPE_BOOL), .data._int = 0});
					pushi({.inst = GOTO, .data._int = readLoop()});
				}
			;

swap		: S_SWAP '(' expr ',' expr ')' {
					pushi({.inst = SWAPV});
				}
			;

inc_dec		: expr '+' '+' {
					pushi({.inst = DUP});
					pushi({.inst = LOAD, .type = type(TYPE_INT), .data._int = 1});
					pushi({.inst = ADD});
					pushi({.inst = ASSIGNV});
				}
			| expr '-' '-' {
					pushi({.inst = DUP});
					pushi({.inst = LOAD, .type = type(TYPE_INT), .data._int = 1});
					pushi({.inst = SUB});
					pushi({.inst = ASSIGNV});
				}
			;

throw		: S_THROW L_STRING {
					pushi({.inst = THROW, .data._ptr = $2});
				}
			;

/* Basic Expressions */

num_op		: expr '+' expr { pushi({.inst = ADD}); }
			| expr '-' expr { pushi({.inst = SUB}); }
			| expr '*' expr { pushi({.inst = MUL}); }
			| expr '/' expr { pushi({.inst = DIV}); }
			| expr '%' expr { pushi({.inst = MOD}); }
			| expr '^' expr { pushi({.inst = POW}); }
			| '-' expr { pushi({.inst = NEG}); } %prec O_UMINUS
			;

bool_op		: expr O_EQ expr { pushi({.inst = EQ}); }
			| expr O_NE expr { pushi({.inst = EQ}); pushi({.inst = NOT}); }
			| expr '>' expr { pushi({.inst = GT}); }
			| expr '<' expr { pushi({.inst = LT}); }
			| expr O_GTE expr { pushi({.inst = GTE}); }
			| expr O_LTE expr { pushi({.inst = LTE}); }
			| '!' expr { pushi({.inst = NOT}); }
			| expr O_AND expr { pushi({.inst = AND}); }
			| expr O_OR expr { pushi({.inst = OR}); }
			;

cast		: '(' type ')' expr { pushi({.inst = CAST}); } %prec O_CAST
			;

access		: expr '.' IDENTIFIER {
					pushi({.inst = ACCESS, .data._ptr = $3});
				}
			;

copy		: E_COPY '(' expr ')' {
					pushi({.inst = COPY}); 
				}
			;

/* Arrays */

arr_init_list		: /* Nothing */
					| expr arr_init_list_elem {
							Value obj = pop().elem;
							obj.data._int++;
							push(toElem(obj));
						}
					;

arr_init_list_elem	: /* Nothing */
					| ',' expr arr_init_list_elem {
							Value obj = pop().elem;
							obj.data._int++;
							push(toElem(obj));
						}
					;

arr_init	: E_NEW type '[' expr ']' {
					pushi({.inst = ARRAYI});
				} %prec O_ARR_INIT
			| E_NEW type '[' expr ']' E_WITH expr {
					pushi({.inst = ARRAYIW});
				} %prec O_ARR_INIT
			| E_NEW type O_EB {
					push(toElem((Value){ .data._int = 0 }));
				} '{' arr_init_list '}' {
					pushi({.inst = ARRAYIL, .data._int = pop().elem.data._int});
				} %prec O_ARR_INIT
			;

arr_get		: expr '[' expr ']' {
					pushi({.inst = ARRAYG});
				} %prec O_ARR_GET
			;

/* Functions */

largs_list	: /* Nothing */
			| type {
					pushi({.inst = APPENDT});
				} IDENTIFIER {
					pushi({.inst = LOADA, .data._ptr = $3});
				} lal_elem 
			;

lal_elem		: /* Nothing */
			| ',' type {
					pushi({.inst = APPENDT});
				} IDENTIFIER {
					pushi({.inst = LOADA, .data._ptr = $4});
				} lal_elem
			;

elambda		: 	{
					// We can use swap here if needed
					pushi({.inst = LOADT, .type = type(TYPE_FUNC)});
				} type {
					pushi({.inst = APPENDT});
				} '(' largs_list ')' {
					pushLoc();
					pushi({});
				} block {
					pushi({.inst = ENDF});
					setInst((Inst){.inst = SAVEF, .data._int = instsCount}, popLoc(), scope);
				}
			;

declaref	: S_FUNC {
					pushi({.inst = LOADT, .type = type(TYPE_UNKNOWN)});
					pushi({.inst = LOADT, .type = type(TYPE_FUNC)});
				} type {
					pushi({.inst = APPENDT});
				} IDENTIFIER '(' largs_list ')' {
					pushLoc();
					pushi({});
				} block {
					pushi({.inst = ENDF});
					setInst((Inst){.inst = SAVEF, .data._int = instsCount}, popLoc(), scope);
					pushi({.inst = SAVEV, .data._ptr = $5});
				}
			;

iargs_list	: /* Nothing */
			| expr {
					pushi({.inst = SWAP});
				} ial_elem
			;

ial_elem	: /* Nothing */
			| ',' expr {
					pushi({.inst = SWAP});
				} ial_elem
			;

invoke_s	: expr '(' iargs_list ')' {
					pushi({.inst = CALLF, .type = type(TYPE_VOID)});
				}
			;

invoke_e	: expr '(' iargs_list ')' {
					pushi({.inst = CALLF, .type = type(TYPE_UNKNOWN)});
				}
			;

return		: S_RETURN {
					pushi({.inst = ENDF});
				}
			| S_RETURN expr {
					pushi({.inst = ENDF});
				}
			;

extern		: S_EXTERN {
					// Push a dummy item on the stack to swap down
					pushi({.inst = LOAD, .type = type(TYPE_INT), .data._int = 0});
				} '(' iargs_list ')' {
					pushi({.inst = EXTERN});
				}
			;

/* If */

if			: if_block
			| else_block
			;

if_cond		: S_IF '(' expr ')' {
					pushLoc();
					pushi({});
				}
			;

if_block	: if_cond block {
					setInst((Inst){.inst = IFN, .data._int = instsCount}, popLoc(), scope);
				}
			;

else_block	: if_cond block {
					int loc = popLoc();
					
					pushLoc();
					pushi({});
					
					setInst((Inst){.inst = IFN, .data._int = instsCount}, loc, scope);
				} S_ELSE block {
					setInst((Inst){.inst = GOTO, .data._int = instsCount}, popLoc(), scope);
				}
			;

/* Utility */

utility_in	: /* Nothing */
			| utility_in declare EOL
			| utility_in declaref
			| utility_in object
			| utility_in fwd_object EOL
			| utility_in EOL
			;

utility		: S_UTILITY IDENTIFIER {
					pushi({.inst = LOADA, .data._ptr = $2});
					pushLoc();
					pushi({});
				} '{' utility_in '}' {
					setInst((Inst){.inst = STARTU, .data._int = instsCount}, popLoc(), scope);
				}
			;

/* Object */

object_in	: /* Nothing */
			| object_in declare EOL
			| object_in declaref
			| object_in EOL
			;

object		: S_OBJECT IDENTIFIER {
					pushi({.inst = LOADA, .data._ptr = $2});
					pushLoc();
					pushi({});
				} '{' object_in '}' {
					setInst((Inst){.inst = STARTO, .data._int = instsCount}, popLoc(), scope);
				}
			;

fwd_object	: S_OBJECT IDENTIFIER {
					pushi({.inst = FWDO, .data._ptr = $2});
				}
			;

init_object	: E_NEW type {
					pushi({.inst = NEWO});
				}
			;

%%

int main(int argc, char** argv) {
	// Check args
	if (argc <= 1) {
		fprintf(stderr, "Arg Error: Must provide file name as argument.\n");
		exit(-1);
	}
	
	// Read file
	yyin = fopen(argv[1], "r+");
	if (yyin == NULL) {
        fprintf(stderr, "Arg Error: Could not open file `%s`.\n", argv[1]);
		exit(-1);
    }
	
	// Check for flags
	bool showByteCode = false;
	if (argc >= 3) {
		showByteCode = strcmp(argv[2], "-b") == 0;
		showCount = strcmp(argv[2], "-c") == 0;
	}
	
	// Start parser
	bc_init();
	
	// Actually lex/parse
	int result = yyparse();
	pushInst((Inst){.inst = -1}, -1); // "end" instruction

	// Interpret
	bc_run(showByteCode);
	bc_end();
	
	return result;
}

#undef pushi