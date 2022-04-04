%code requires {
	#include "../bytecode.h"
}

%{
	#include <stdlib.h>
	#include <stdio.h>
	#include <string.h>
	
	FILE* yyin;
	int scope = 0;
	
	#define pushi(...) pushInst((Inst) __VA_ARGS__, scope);
	
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
}

/* Important */
%token EOL
%token<v_str> IDENTIFIER

/* Statement keywords */
%token S_EXTERN S_IF S_ELSE S_WHILE S_RETURN S_BREAK S_FUNC S_SWAP S_FOR

/* Expression keywords */
%token E_NEW

/* Built-in types & literals */
%token T_AUTO T_VOID T_STR T_INT T_BOOL T_FLOAT T_FUNC
%token <v_int> L_NUMBER L_BOOL
%token <v_str> L_STRING 
%token <v_float> L_FLOAT

/* Operators */
%token T_EQ T_NE T_AND T_OR T_GTE T_LTE
%left T_AND T_OR
%left '>' '<' T_EQ T_NE T_GTE T_LTE
%left '+' '-'
%left '*' '/' '%'
%left '!' O_UMINUS
%left O_CAST

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
			| T_FUNC {
					pushi({.inst = LOADT, .type = type(TYPE_FUNC)});
				} '(' type_list ')'
			| type '[' ']' {
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
			;

statement	: declare
			| assign
			| invoke_s
			| return
			| break
			| extern
			| swap
			| inc_dec
			| arr_save
			;

expr		: '(' expr ')'
			| L_NUMBER { pushi({.inst = LOAD, .type = type(TYPE_INT), .a.v_int = $1}); }
			| L_STRING { pushi({.inst = LOAD, .type = type(TYPE_STR), .a.v_ptr = $1}); }
			| L_BOOL { pushi({.inst = LOAD, .type = type(TYPE_BOOL), .a.v_int = $1}); }
			| L_FLOAT { pushi({.inst = LOAD, .type = type(TYPE_FLOAT), .a.v_float = $1}); }
			| IDENTIFIER { pushi({.inst = LOADV, .a.v_ptr = $1}); }
			| int_op
			| bool_op
			| cast
			| elambda /* Explict Lambda */
			| invoke_e
			| extern
			| arr_init
			| arr_get
			| arr_length
			;

/* Basic Statements */

declare		: type IDENTIFIER '=' expr { pushi({.inst = SAVEV, .a.v_ptr = $2}); }
			| T_AUTO IDENTIFIER {
					pushi({.inst = LOADT, .type = type(TYPE_UNKNOWN)});
				} '=' expr {
					pushi({.inst = SAVEV, .a.v_ptr = $2});
				}
			;

assign		: IDENTIFIER '=' expr { 
					pushi({.inst = RESAVEV, .a.v_ptr = $1}); 
				}
			;

arr_save	: IDENTIFIER '[' expr ']' '=' expr {
					pushi({.inst = ARRAYS, .a.v_ptr = $1});
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
					setInst((Inst){.inst = IFN, .a.v_int = instsCount + 1}, popLoc(), scope);
					pushi({.inst = GOTO, .a.v_int = popLoc()});
				}
			;

for			: S_FOR '(' {
					scope++;
				} declare ',' {
					pushLoc();
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
					setInst((Inst){.inst = IFN, .a.v_int = instsCount + 1}, popLoc(), scope);
					pushi({.inst = GOTO, .a.v_int = popLoc()});
					scope--;
				}
			;

break		: S_BREAK {
					pushi({.inst = LOAD, .type = type(TYPE_BOOL), .a.v_int = 0});
					pushi({.inst = GOTO, .a.v_int = readLoop()});
				}
			;

swap		: S_SWAP '(' IDENTIFIER ',' IDENTIFIER ')' {
					pushi({.inst = LOADV, .a.v_ptr = $3});
					pushi({.inst = LOADV, .a.v_ptr = $5});
					pushi({.inst = RESAVEV, .a.v_ptr = $3});
					pushi({.inst = RESAVEV, .a.v_ptr = $5});
				}
			;

inc_dec		: IDENTIFIER '+' '+' {
					pushi({.inst = LOADV, .a.v_ptr = $1});
					pushi({.inst = LOAD, .type = type(TYPE_INT), .a.v_int = 1});
					pushi({.inst = ADD});
					pushi({.inst = RESAVEV, .a.v_ptr = $1});
				}
			| IDENTIFIER '-' '-' {
					pushi({.inst = LOADV, .a.v_ptr = $1});
					pushi({.inst = LOAD, .type = type(TYPE_INT), .a.v_int = 1});
					pushi({.inst = SUB});
					pushi({.inst = RESAVEV, .a.v_ptr = $1});
				}
			;

/* Basic Expressions */

int_op		: expr '+' expr { pushi({.inst = ADD}); }
			| expr '-' expr { pushi({.inst = SUB}); }
			| expr '*' expr { pushi({.inst = MUL}); }
			| expr '/' expr { pushi({.inst = DIV}); }
			| expr '%' expr { pushi({.inst = MOD}); }
			| '-' expr { pushi({.inst = NEG}); } %prec O_UMINUS
			;

bool_op		: expr T_EQ expr { pushi({.inst = EQ}); }
			| expr T_NE expr { pushi({.inst = EQ}); pushi({.inst = NOT}); }
			| expr '>' expr { pushi({.inst = GT}); }
			| expr '<' expr { pushi({.inst = LT}); }
			| expr T_GTE expr { pushi({.inst = GTE}); }
			| expr T_LTE expr { pushi({.inst = LTE}); }
			| '!' expr { pushi({.inst = NOT}); }
			| expr T_AND expr { pushi({.inst = AND}); }
			| expr T_OR expr { pushi({.inst = OR}); }
			;

cast		: '(' T_STR ')' expr { pushi({.inst = CASTS}); } %prec O_CAST
			;

arr_init	: E_NEW type '[' expr ']' {
					pushi({.inst = ARRAYI});
				}
			;

arr_get		: IDENTIFIER '[' expr ']' {
					pushi({.inst = ARRAYG, .a.v_ptr = $1});
				}
			;

arr_length	: IDENTIFIER '.' IDENTIFIER {
					if (strcmp($3, "length") != 0) {
						yyerror("Accessor must be `length` (for now).");
					}

					pushi({.inst = ARRAYL, .a.v_ptr = $1});
				}
			;

/* Functions */

largs_list	: /* Nothing */
			| type {
					pushi({.inst = APPENDT});
				} IDENTIFIER {
					pushi({.inst = LOADA, .a.v_ptr = $3});
				} lal_elem 
			;

lal_elem		: /* Nothing */
			| ',' type {
					pushi({.inst = APPENDT});
				} IDENTIFIER {
					pushi({.inst = LOADA, .a.v_ptr = $4});
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
					setInst((Inst){.inst = SAVEF, .a.v_int = instsCount}, popLoc(), scope);
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
					setInst((Inst){.inst = SAVEF, .a.v_int = instsCount}, popLoc(), scope);
					pushi({.inst = SAVEV, .a.v_ptr = $5});
				}
			;

iargs_list	: /* Nothing */
			| expr ial_elem
			;

ial_elem	: /* Nothing */
			| ',' expr ial_elem
			;

invoke_s	: IDENTIFIER '(' iargs_list ')' {
					pushi({.inst = CALLF, .a.v_ptr = $1, .type = type(TYPE_VOID)});
				}
			;

invoke_e	: IDENTIFIER '(' iargs_list ')' {
					pushi({.inst = CALLF, .a.v_ptr = $1, .type = type(TYPE_UNKNOWN)});
				}
			;

return		: S_RETURN {
					pushi({.inst = ENDF});
				}
			| S_RETURN expr {
					pushi({.inst = ENDF});
				}
			;

extern		: S_EXTERN '(' iargs_list ')' {
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
					setInst((Inst){.inst = IFN, .a.v_int = instsCount}, popLoc(), scope);
				}
			;

else_block	: if_cond block {  
					int loc = popLoc();
					
					pushLoc();
					pushi({});
					
					setInst((Inst){.inst = IFN, .a.v_int = instsCount}, loc, scope);
				} S_ELSE block {
					setInst((Inst){.inst = GOTO, .a.v_int = instsCount}, popLoc(), scope);
				}
			;

%%

#undef pushi

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
	bool showCount = false;
	if (argc >= 3) {
		showByteCode = strcmp(argv[2], "-b") == 0;
		showCount = strcmp(argv[2], "-c") == 0;
	}
	
	// Start parser
	bc_init();
	int result = yyparse();
	bc_run(showByteCode, showCount);
	bc_end();
	
	return result;
}