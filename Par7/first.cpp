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
/// This file defines some functions to work with sets of symbols in the grammar.
///
/// \author Yuriy Solodkyy <yuriy.solodkyy@gmail.com>
///
/// \see https://github.com/solodon4/Par7
/// \see https://github.com/solodon4/SELL
///

#include <map>
#include <set>
#include <string>
#include "ebnfdefs.hpp"

//------------------------------------------------------------------------------

non_terminal_set rhs_nonterminals(symbol& s)
{
    non_terminal_set result;

    if (non_terminal nt = s.pointer()->is_non_terminal())
    {
        result.insert(std::move(nt));
    }

    return result;
}

non_terminal_set rhs_nonterminals(Production& p)
{
    non_terminal_set result;

    for (auto& s : p.second)
    {
        non_terminal_set tmp = rhs_nonterminals(s);
        result.insert(tmp.begin(), tmp.end());
    }

    return result;
}

non_terminal_set rhs_nonterminals(Grammar& grammar)
{
    non_terminal_set result;

    for (auto& p : grammar.productions())
    {
        non_terminal_set tmp = rhs_nonterminals(p);
        result.insert(tmp.begin(), tmp.end());
    }

    return result;
}

//------------------------------------------------------------------------------

non_terminal_set lhs_nonterminals(Grammar& grammar)
{
    non_terminal_set result;

    for (auto& p : grammar.productions())
    {
        result.insert(p.first);
    }

    return result;
}

//------------------------------------------------------------------------------

non_terminal_set empty_nonterminals(Grammar& grammar)
{
    non_terminal_set result;

    for (auto& p : grammar.productions())
    {
        if (p.second.empty())
        {
            result.insert(p.first);
        }
    }

    return result;
}

//------------------------------------------------------------------------------

terminal_set first(
                    Grammar& g,
                    symbol_sequence::const_iterator beg,
                    symbol_sequence::const_iterator end,
                    const std::map<non_terminal, terminal_set>& current,
                    const non_terminal_set& empty_nt
             )
{
    terminal_set result;

    if (beg==end) // This is production of the form: X ->
    {
        result.insert(g.epsilon());
    }
    else
    for (symbol_sequence::const_iterator p = beg; p != end; ++p)
    {
        const auto& x = *p;

        if (non_terminal n = x.pointer()->is_non_terminal())
        {
            std::map<non_terminal, terminal_set>::const_iterator p = current.find(n);

            if (p != current.end())
            {
                const terminal_set& s = p->second;
                result.insert(s.begin(), s.end());
            }

            if (empty_nt.find(n) == empty_nt.end())
            {
                break;
            }
        }
        else
        if (terminal t = x.pointer()->is_terminal())
        {
            result.insert(t);
            break;
        }
    }

    return result;
}

std::map<non_terminal, terminal_set> first(Grammar& grammar)
{
    const non_terminal_set empty_nt = empty_nonterminals(grammar);
    std::map<non_terminal, terminal_set> result;
    non_terminal_set keys = rhs_nonterminals(grammar);

    for (const auto& n : keys)
    {
        result[n] = terminal_set();
    }

    bool changes = false;

    do
    {
        changes = false;

        for (auto& p : grammar.productions())
        {
            size_t size_before = result[p.first].size();
            terminal_set f = first(grammar, p.second.begin(), p.second.end(), result, empty_nt);
            result[p.first].insert(f.begin(), f.end());

            if (!changes && size_before != result[p.first].size())
            {
                changes = true;
            }
        }
    }
    while (changes);

    return result;
}

//------------------------------------------------------------------------------

std::map<non_terminal, terminal_set> follow(Grammar& grammar)
{
    const non_terminal_set empty_nt = empty_nonterminals(grammar);
    const std::map<non_terminal, terminal_set> first_sets = first(grammar);
    std::map<non_terminal, terminal_set> result;
    non_terminal_set keys = rhs_nonterminals(grammar);

    for (const auto& n : keys)
    {
        result[n] = terminal_set();
    }

    bool changes = false;

    result[grammar.start_symbol()].insert(grammar.eof()); // First put $ (the end of input marker) in Follow(S) (S is the start symbol)

    do
    {
        changes = false;

        for (auto& p : grammar.productions())
        {
            // If there is a production A → aBb, (where a can be a whole string) then everything in FIRST(b) except for ε is placed in FOLLOW(B).
            for (auto i = p.second.begin(); i != p.second.end(); ++i)
            {
                if (non_terminal n = i->pointer()->is_non_terminal())
                {
                    size_t size_before = result[n].size();
                    auto j = i+1;

                    if (j == p.second.end())
                    {
                        // If there is a production A → aB, then everything in FOLLOW(A) is in FOLLOW(B)
                        result[n] = join(result[n], result[p.first]);
                    }
                    else
                    {
                        // If there is a production A → aBb, (where a can be a whole string) then everything in FIRST(b) except for ε is placed in FOLLOW(B).
                        terminal_set fs = first(grammar, j, p.second.end(), first_sets, empty_nt);
                        terminal_set::const_iterator q = fs.find(grammar.epsilon());

                        if (q != fs.end())
                        {
                            // If there is a production A → aBb, where FIRST(b) contains ε, then everything in FOLLOW(A) is in FOLLOW(B)
                            result[n] = join(result[n], result[p.first]);
                            fs.erase(q);
                        }

                        result[n] = join(result[n], fs);

                    }

                    if (!changes && size_before != result[n].size())
                    {
                        changes = true;
                    }
                }
            }
        }
    }
    while (changes);
    
    return result;
}

//------------------------------------------------------------------------------
