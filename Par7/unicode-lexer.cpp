//
//  unicode-lexer.cpp
//  Par7
//
//  Created by Yuriy Solodkyy on 3/12/15.
//  Copyright (c) 2015 CrowdsTech. All rights reserved.
//

#include <stdio.h>
#include <ctype.h>

#include "ebnfdefs.hpp"
#include "ebnf.tab.hpp"

char yytext[1024];
extern int g_line;

int yylex()
{
    int c;
    while((c = getchar()) != EOF)
    {
        if (c == ' ' || c == '\t')
        {
        }
        else
        if (c == '\n')
        {
            g_line++;
        }
        else
        if (c == '\r')
        {
        }
        else
        if (isalpha(c))
        {
            char* yyptr = yytext;
            while (isalpha(c) || isdigit(c))
            {
                *yyptr++ = c;
                c = getchar();
            }
            *yyptr = 0;
            yylval.m_str = new std::string(yytext);
            //printf("Lexer: '%s'\n",yytext);
            return ID;
        }
        else
        if (c == '"')
        {
            char* yyptr = yytext;
            *yyptr++ = c;
            while ((c = getchar()) != EOF && c != '"')
            {
                *yyptr++ = c;
            }
            if (c == '"') *yyptr++ = c;
            *yyptr = 0;
            yylval.m_str = new std::string(yytext);
            //printf("Lexer: '%s'\n",yytext);
            return STR;
        }
        else
        {
            //printf("Lexer: '%c'\n",c);
            return c;
        }
    }

    //printf("Lexer: 'EOF'\n");
    return 0;
}
