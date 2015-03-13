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
    //    YYSTYPE() { new(&m_term) Term(); }  // FIX: try to make this work without keeping pointers only
    //    ~YYSTYPE();
    //    YYSTYPE& operator=(const YYSTYPE&);
    Term*         m_term;
    Def*          m_def;
    Alternatives* m_alt;
    Rule*         m_rule;
    Syntax*       m_syntax;
    std::string*  m_str;
    int           m_int;
}

%start syntax

%token <m_str> ID "identifier"
%token <m_str> STR "string"
%type  <m_term> term
%type  <m_def>  def
%type  <m_alt>  alternatives
%type  <m_rule>        rule
%type  <m_syntax>      syntax

%printer    { std::clog << $$/* << ':' << *yylval.m_str*/; } ID STR
%destructor { delete $$; } ID STR

%%

syntax
    : syntax rule           { $$ = $1; $$->emplace_back(std::move(*$2)); }
    | /* empty */           { extern Syntax* grammar; grammar = $$ = new Syntax; }
	;

rule
    : ID '=' alternatives ';' { $$ = new Rule(std::move(*$1), std::move(*$3)); }
	;

alternatives
    : def '|' alternatives  { $$ = $3;               $$->prepend(std::move(*$1)); }
    | def                   { $$ = new Alternatives; $$->prepend(std::move(*$1)); }
    | /* empty */           { $$ = new Alternatives; }
    ;

    def : term def          { $$ = $2; $$->prepend(std::unique_ptr<Term>($1)); }
    | /* empty */           { $$ = new Def; }
    ;

term
    : ID                    { $$ = new NonTerminal(*$1); }
    | STR                   { $$ = new    Terminal(*$1); }
    ;

%%
