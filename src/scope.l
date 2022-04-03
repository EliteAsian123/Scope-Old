%{
	#include "scope.tab.h"
	#include "../unescape.h"
%}

%x BIG_COMMENT
%x COMMENT
%x INSERT

%%

"/*"				{ BEGIN(BIG_COMMENT); }
<BIG_COMMENT>"*/"	{ BEGIN(INITIAL); }
<BIG_COMMENT>\n		{ }
<BIG_COMMENT>.		{ }

"//"			{ BEGIN(COMMENT); }
<COMMENT>\n 	{ BEGIN(INITIAL); }
<COMMENT>.		{ }

"#insert "		{ BEGIN(INSERT); }
<INSERT>.* {
	// Read file
	yyin = fopen(yytext, "r+");
	if (yyin == NULL) {
		fprintf(stderr, "Lex Error: Could not open file `%s`.\n", yytext);
		yyterminate();
	}
	
	// Finish
	yypush_buffer_state(yy_create_buffer(yyin, YY_BUF_SIZE));
	BEGIN(INITIAL);
}

"extern"		{ return S_EXTERN; }
"if"			{ return S_IF; }
"else"			{ return S_ELSE; }
"while"			{ return S_WHILE; }
"ret"			{ return S_RETURN; }
"break"			{ return S_BREAK; }
"func"			{ return S_FUNC; }
"swap"			{ return S_SWAP; }
"for"			{ return S_FOR; }

"new"			{ return E_NEW; }

"auto"			{ return T_AUTO; }
"void"			{ return T_VOID; }
"int" 			{ return T_INT; }
"bool"			{ return T_BOOL; }
"string"		{ return T_STR; }
"float"			{ return T_FLOAT; }
"function"		{ return T_FUNC; }

"=="			{ return T_EQ; }
"!="			{ return T_NE; }
"&&"			{ return T_AND; }
"||"			{ return T_OR; }
">="			{ return T_GTE; }
"<="			{ return T_LTE; }

\"(\\.|[^"\\])*\" {
	// Copy, remove quotes, and unescape
	
	size_t len = strlen(yytext) - 1;
	char* in = strndup(yytext + 1, len);
	in[len - 1] = '\0';
	
	char* out = malloc(len);
	if (unescape(in, out, len) == NULL) {
		fprintf(stderr, "Lex Error: Failed to unescape.\n");
		exit(-1);
	}
	
	len = strlen(out) + 1;
	out = realloc(out, len);
	out[len - 1] = '\0';
	
	yylval.v_str = out;
	
	return L_STRING;
}
[0-9]* {
	yylval.v_int = atoi(yytext);
	return L_NUMBER;
}
([0-9]*[.])?[0-9]+f {
	char* str = strndup(yytext, strlen(yytext));
	yylval.v_float = atof(str);
	free(str);

	return L_FLOAT;
}
"true" {
	yylval.v_int = 1;
	return L_BOOL;
}
"false" {
	yylval.v_int = 0;
	return L_BOOL;
}
[a-zA-Z_][a-zA-Z0-9_]* {
	yylval.v_str = strdup(yytext);
	return IDENTIFIER;
}

";"				{ return EOL; }
[ \t\r\n]+ 		{ }
. 				{ return yytext[0]; }
<<EOF>> {
	yypop_buffer_state();

	if (!YY_CURRENT_BUFFER) {
		yyterminate();
	}
}

%%