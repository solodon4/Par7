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

std::set<non_terminal> rhs_nonterminals(symbol& s)
{
    std::set<non_terminal> result;

    if (NonTerminal* nt = dynamic_cast<NonTerminal*>(&s))
    {
        result.insert(non_terminal(std::move(nt)));
    }

    return result;
}

std::set<non_terminal> rhs_nonterminals(Production& p)
{
    std::set<non_terminal> result;

    for (auto& s : p.rhs)
    {
        std::set<non_terminal> tmp = rhs_nonterminals(s);
        result.insert(tmp.begin(), tmp.end());
    }

    return result;
}

std::set<non_terminal> rhs_nonterminals(Grammar& grammar)
{
    std::set<non_terminal> result;

    for (auto& p : grammar.productions())
    {
        std::set<non_terminal> tmp = rhs_nonterminals(p.second);
        result.insert(tmp.begin(), tmp.end());
    }

    return result;
}

//------------------------------------------------------------------------------

std::set<non_terminal> lhs_nonterminals(Grammar& grammar)
{
    std::set<non_terminal> result;

    for (auto& p : grammar.productions())
    {
        result.insert(p.first);
    }

    return result;
}

//------------------------------------------------------------------------------

std::set<non_terminal> empty_nonterminals(Grammar& grammar)
{
    std::set<non_terminal> result;

    for (auto& p : grammar.productions())
    {
        if (p.second.rhs.empty())
        {
            result.insert(p.second.lhs);
        }
    }

    return result;
}

//------------------------------------------------------------------------------

std::set<terminal> first(
                    Grammar& g,
                    Production& p,
                    const std::map<non_terminal, std::set<terminal>>& current,
                    const std::set<non_terminal>& empty_nt
                    )
{
    std::set<terminal> result;

    if (p.rhs.empty()) // This is production of the form: X ->
    {
        result.insert(g.epsilon());
    }
    else
    for (const auto& x : p.rhs)
    {
        if (NonTerminal* n = dynamic_cast<NonTerminal*>(x.pointer()))
        {
            std::map<non_terminal, std::set<terminal>>::const_iterator p = current.find(non_terminal(n));

            if (p != current.end())
            {
                const std::set<terminal>& s = p->second;
                result.insert(s.begin(), s.end());
            }

            if (empty_nt.find(non_terminal(n)) == empty_nt.end())
            {
                break;
            }
        }
        else
        if (Terminal* t = dynamic_cast<Terminal*>(x.pointer()))
        {
            result.insert(terminal(t));
            break;
        }
    }

    return result;
}

std::map<non_terminal, std::set<terminal>> first(Grammar& grammar)
{
    const std::set<non_terminal> empty_nt = empty_nonterminals(grammar);
    std::map<non_terminal, std::set<terminal>> result;
    std::set<non_terminal> keys = rhs_nonterminals(grammar);

    for (const auto& n : keys)
    {
        result[n] = std::set<terminal>();
    }

    bool changes = false;

    do
    {
        changes = false;

        for (auto& p : grammar.productions())
        {
            size_t size_before = result[p.first].size();
            std::set<terminal> f = first(grammar, p.second, result, empty_nt);
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

std::map<non_terminal, std::set<terminal>> follow(Grammar& grammar)
{
    const std::set<non_terminal> empty_nt = empty_nonterminals(grammar);
    const std::map<non_terminal, std::set<terminal>> first_sets = first(grammar);
    std::map<non_terminal, std::set<terminal>> result;
    std::set<non_terminal> keys = rhs_nonterminals(grammar);

    for (const auto& n : keys)
    {
        result[n] = std::set<terminal>();
    }

    bool changes = false;

    result[grammar.start_symbol()].insert(grammar.eof()); // First put $ (the end of input marker) in Follow(S) (S is the start symbol)

    do
    {
        changes = false;

        for (auto& p : grammar.productions())
        {
            size_t size_before = result[p.first].size();
            std::set<terminal> f = first(grammar, p.second, result, empty_nt);
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
