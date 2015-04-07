//
//  Par7: Parser Generator Library for C++
//
//  Copyright 2015 Yuriy Solodkyy.
//  All rights reserved.
//
//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted provided that the following conditions are met:
//
//      * Redistributions of source code must retain the above copyright
//        notice, this list of conditions and the following disclaimer.
//
//      * Redistributions in binary form must reproduce the above copyright
//        notice, this list of conditions and the following disclaimer in the
//        documentation and/or other materials provided with the distribution.
//
//      * Neither the names of Mach7 project nor the names of its contributors
//        may be used to endorse or promote products derived from this software
//        without specific prior written permission.
//
//  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
//  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
//  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
//  IN NO EVENT SHALL THE COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY
//  DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
//  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
//  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
//  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
//  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
//  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

///
/// \file
///
/// Grammar for EBNF grammar definition
/// http://en.wikipedia.org/wiki/Extended_Backus–Naur_Form
/// http://standards.iso.org/ittf/PubliclyAvailableStandards/s026153_ISO_IEC_14977_1996(E).zip
///
/// \author Yuriy Solodkyy <yuriy.solodkyy@gmail.com>
///
/// \see https://github.com/solodon4/Par7
/// \see https://github.com/solodon4/SELL
///

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
    //    YYSTYPE() { new(&m_term) Symbol(); }  // FIX: try to make this work without keeping pointers only
    //    ~YYSTYPE();
    //    YYSTYPE& operator=(const YYSTYPE&);
    std::string*         m_str;
    Symbol*              m_term;
    std::vector<symbol>* m_terms;
    Production*          m_production;
    Grammar*             m_grammar;
}

%start grammar

%token <m_str>         ID "identifier"
%token <m_str>         STR "string"
%type  <m_term>        term
%type  <m_terms>       terms
%type  <m_production>  production
%type  <m_grammar>     grammar

%printer    { std::clog << $$/* << ':' << *yylval.m_str*/; } ID STR
%destructor { std::clog << "deleting" << typeid(*$$).name() << std::endl; delete $$; } ID STR

%%

grammar
    : grammar production    { $$ = $1; $$->append(std::move(*$2)); }
    | /* empty */           { grammar = $$ = new Grammar; }
	;

production
    : ID '=' terms ';'      { $$ = new Production(grammar->nonterminal($1->c_str()), std::move(*$3)); delete $3; }
	;

terms : term terms          { $$ = $2; prepend_to(*$2, symbol(std::move($1))); }
    | /* empty */           { $$ = new std::vector<symbol>; }
    ;

term
    : ID                    { $$ = grammar->nonterminal($1->c_str()).pointer(); }
    | STR                   { $$ = grammar->terminal($1->c_str()).pointer(); }
    ;

%%
