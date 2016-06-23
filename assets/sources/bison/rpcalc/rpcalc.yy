%{
/* Prologues here. */
#define YYSTYPE       double
#include <math.h>
int yylex(void);
void yyerror(const char* );
%}

/* Bison declarations starts here */
%token NUM

%%
/* Grammar rules and actions follow.  */

%%

/* Epilogue here */
