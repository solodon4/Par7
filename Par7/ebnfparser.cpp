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
/// This file defines main entry point to EBNF parser.
///
/// \author Yuriy Solodkyy <yuriy.solodkyy@gmail.com>
///
/// \see https://github.com/solodon4/Par7
/// \see https://github.com/solodon4/SELL
///

#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <sstream>

#include "ebnfdefs.hpp"
#include "utf-8-reader.hpp"

//------------------------------------------------------------------------------

extern int yyparse(void);
extern int yy_flex_debug;
extern int yydebug;
Grammar* grammar = nullptr;

//------------------------------------------------------------------------------

int main(int argc, char* argv[])
{
    yydebug = 1;       // disable Yacc debugging mode
    //yy_flex_debug = 1; // disable Flex debugging mode
    std::string base_name = "undefined";

    try
    {
        if (argc < 2)
        {
            std::cerr << "Usage: " << argv[0] << " filename.ebnf" << std::endl;
            return 1;
        }

#if 0
        std::fstream binfile(argv[1], std::ios::in | std::ios::binary | std::ios::out);

        const size_t N = 256;
        char buffer[N];
        unicode::code_point result[N];

        while (binfile)
        {
            size_t n = N;

            binfile.read(buffer, 256);
            std::streamsize bytes = binfile.gcount(); // Number of bytes actually read

            unicode::utf8decode(buffer, bytes, result, n);

            for (size_t i = 0; i < N-n; ++i)
            {
                std::cout << "\\U" << result[i] << std::endl;
            }
            
        }
        
        binfile.close();
        return 0;
#else
        if (!freopen(argv[1],"r",stdin)) //redirect standard input
        {
            std::cerr << "Error: Can't open file " << argv[1] << std::endl;
            return 2;
        }

        base_name = argv[1];
        base_name.erase(base_name.find(".ebnf"));

        int result = yyparse();
        std::clog.flush();

        if (result == 0)
        {
            assert(grammar); // Must be valid object after successful parsing
            std::cout << "Resulting Grammar:" << std::endl << *grammar << std::endl;

            extern std::set<non_terminal> rhs_nonterminals(Grammar&);

            std::cout << std::endl << "RHS non-terminals: ";
            std::set<non_terminal> rhs_nt = rhs_nonterminals(*grammar);
            separated_output(std::cout, rhs_nt, ",");

            extern std::set<non_terminal> lhs_nonterminals(Grammar&);

            std::cout << std::endl << "LHS non-terminals: ";
            std::set<non_terminal> lhs_nt = lhs_nonterminals(*grammar);
            separated_output(std::cout, lhs_nt, ",");

            std::cout << std::endl << "Undefined non-terminals: ";
            separated_output(std::cout, difference(rhs_nt, lhs_nt), ",");

            std::cout << std::endl << "Empty non-terminals: ";
            extern std::set<non_terminal> empty_nonterminals(Grammar& grammar);
            separated_output(std::cout, empty_nonterminals(*grammar), ",");

            std::cout << std::endl << "First sets: " << std::endl;
            extern std::map<non_terminal, std::set<terminal>> first(Grammar& grammar);
            auto firstsets = first(*grammar);
            separated_output(std::cout,firstsets,"\n");
            std::cout << std::endl;

            std::cout << std::endl << "Follow sets: " << std::endl;
            extern std::map<non_terminal, std::set<terminal>> follow(Grammar& grammar);
            auto followsets = follow(*grammar);
            separated_output(std::cout,followsets,"\n");
            std::cout << std::endl;
        }

        return result;
#endif
    }
    catch (...)
    {
        std::cerr << "ERROR: Unhandled exception caught while parsing" << std::endl;
    	return 4;
    }
}

//------------------------------------------------------------------------------
