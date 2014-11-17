/*
	Grammar for EBNF grammar definition
    http://en.wikipedia.org/wiki/Extended_Backus–Naur_Form
    http://standards.iso.org/ittf/PubliclyAvailableStandards/s026153_ISO_IEC_14977_1996(E).zip
*/

%{
#include "ebnfdefs.hpp"

using namespace std;

int  yyparse(void);
int  yylex(void);
void yyerror(const char *str)
{
    extern int g_line;
    extern char* yytext;
    cerr << "error[" << g_line << "]: " << str << endl;
    cerr << "before symbol: " << yytext << endl;
}

/* Enabling traces.  */
#define YYDEBUG 1

/* Enabling verbose error messages.  */
#define YYERROR_VERBOSE 1

/* Enabling the token table.  */
#define YYTOKEN_TABLE 1
%}

%union
{
    std::string* m_str;
    int          m_int;
}

%start syntax

%token <m_str> ID "identifier"
%token <m_str> STR "string"

%printer    { std::clog << *$$; } ID
%destructor { delete $$; } ID

%%

syntax
    : syntax rule
    | /* empty */
	;

rule
    : ID '=' definitions
	;

definitions
    : def '|' definitions
    | def
    | /* empty */
    ;

def : term def
    | /* empty */
    ;

term
    : ID
    | STR
    ;

%%
