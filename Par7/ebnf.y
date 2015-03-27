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
extern Grammar* grammar;

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
    std::string*                               m_str;
    Term*                                      m_term;
    std::vector<polymorphic<nonowning<Term>>>* m_terms;
    Production*                                m_production;
    Grammar*                                   m_grammar;
}

%start grammar

%token <m_str> ID "identifier"
%token <m_str> STR "string"
%type  <m_term> term
%type  <m_terms> terms
%type  <m_production>  production
%type  <m_grammar>     grammar

%printer    { std::clog << $$/* << ':' << *yylval.m_str*/; } ID STR
%destructor { delete $$; } ID STR

%%

grammar
    : grammar production    { $$ = $1; $$->append(std::move(*$2)); }
    | /* empty */           { grammar = $$ = new Grammar; }
	;

production
    : ID '=' terms ';'      { $$ = new Production(std::move(*$1), std::move(*$3)); delete $3; }
	;

    terms : term terms      { $$ = $2; prepend_to(*$2, polymorphic<nonowning<Term>>(std::move($1))); }
    | /* empty */           { $$ = new std::vector<polymorphic<nonowning<Term>>>; }
    ;

term
    : ID                    { $$ = grammar->nonterminal($1->c_str()); }
    | STR                   { $$ = grammar->terminal($1->c_str()); }
    ;

%%
