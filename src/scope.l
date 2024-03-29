%{
	#include "scope.tab.h"
	#include "../c_escape/c_escape.h"

	char** importedFiles = NULL;
	size_t importedFilesCount = 0;
%}

%x BIG_COMMENT
%x DOC_COMMENT
%x COMMENT
%x INSERT
%x IMPORT

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

"import" 		{ BEGIN(IMPORT); }
<IMPORT>[^ ;\t\r\n]* {
	if (yytext[0] != '\"' || yytext[strlen(yytext) - 1] != '\"') {
		fprintf(stderr, "Lex Error: import statement must be followed by a string.\n");
		yyterminate();
	}
	
	// "TEST.scope"N
	// x    ++++++x+
	char* fileName = calloc(strlen(yytext) - 2 + 6 + 1, 1);
	strncpy(fileName, yytext + 1, strlen(yytext) - 2);
	strcat(fileName, ".scope");
	
	// Skip if the file was already imported
	for (size_t i = 0; i < importedFilesCount; i++) {
		if (strcmp(importedFiles[i], fileName) == 0) {
			BEGIN(INITIAL);
			return EOL; // Just do nothing.
		}
	}
	
	// Add file to list
	importedFilesCount++;
	importedFiles = realloc(importedFiles, sizeof(char*) * importedFilesCount);
	importedFiles[importedFilesCount - 1] = fileName;
	
	// Read file
	yyin = fopen(fileName, "r");
	if (yyin == NULL) {
		fprintf(stderr, "Lex Error: Could not open file `%s`.\n", fileName);
		yyterminate();
	}
	
	// Finish
	yypush_buffer_state(yy_create_buffer(yyin, YY_BUF_SIZE));
	BEGIN(INITIAL);
	return EOL; // Just do nothing.
}
<IMPORT>.		{ }

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
"copy"			{ return E_COPY; }

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

	int errloc = 0;
	char* out = unescape(in, CE_C, &errloc);
	if (out == NULL) {
		fprintf(stderr, "Lex Error: Failed to unescape. Error at character %d.\n", errloc);
		exit(-1);
	}
	
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
"fInf" {
	yylval.v_float = INFINITY;
	return L_FLOAT;
}
"dInf" {
	yylval.v_double = INFINITY;
	return L_DOUBLE;
}
"fNan" {
	yylval.v_float = NAN;
	return L_FLOAT;
}
"dNan" {
	yylval.v_double = NAN;
	return L_DOUBLE;
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