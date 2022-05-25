%{
	#include "scope.tab.h"
	#include "../unescape.h"
%}

%x BIG_COMMENT
%x DOC_COMMENT
%x COMMENT
%x INSERT

%%

"/*"				{ BEGIN(BIG_COMMENT); }
<BIG_COMMENT>"*/"	{ BEGIN(INITIAL); }
<BIG_COMMENT>\n		{ }
<BIG_COMMENT>.		{ }

"/%"				{ BEGIN(DOC_COMMENT); }
<DOC_COMMENT>"%/"	{ BEGIN(INITIAL); }
<DOC_COMMENT>\n		{ }
<DOC_COMMENT>.		{ }

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
"throw"			{ return S_THROW; }
"repeat"		{ return S_REPEAT; }
"utility"		{ return S_UTILITY; }
"object"		{ return S_OBJECT; }

"new"			{ return E_NEW; }
"with"			{ return E_WITH; }

"auto"			{ return T_AUTO; }
"void"			{ return T_VOID; }
"int" 			{ return T_INT; }
"bool"			{ return T_BOOL; }
"string"		{ return T_STR; }
"float"			{ return T_FLOAT; }
"function"		{ return T_FUNC; }
"long"			{ return T_LONG; }
"double"		{ return T_DOUBLE; }

"=="			{ return O_EQ; }
"!="			{ return O_NE; }
"&&"			{ return O_AND; }
"||"			{ return O_OR; }
">="			{ return O_GTE; }
"<="			{ return O_LTE; }
"[]"			{ return O_EB; }

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
[0-9]*l {
	yylval.v_long = atol(yytext);
	return L_LONG;
}
([0-9]*[.])?[0-9]+f {
	char* str = strndup(yytext, strlen(yytext));
	yylval.v_float = atof(str);
	free(str);

	return L_FLOAT;
}
([0-9]*[.])?[0-9]+d {
	char* str = strndup(yytext, strlen(yytext));
	yylval.v_double = atof(str);
	free(str);
	
	return L_DOUBLE;
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