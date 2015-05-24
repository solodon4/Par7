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
/// This file defines main grammar entities: terminals, non-terminals, productions and grammar.
///
/// \author Yuriy Solodkyy <yuriy.solodkyy@gmail.com>
///
/// \see https://github.com/solodon4/Par7
/// \see https://github.com/solodon4/SELL
///

#pragma once

#include "valueptr.hpp"
#include "stlutils.hpp"
#include <string>

//==============================================================================
// Forward declarations
//==============================================================================

// To disable Clang's deprecation warning
#define register

//------------------------------------------------------------------------------

struct    Terminal;
struct NonTerminal;

//------------------------------------------------------------------------------

struct Symbol : std::string
{
    using std::string::string;
    virtual ~Symbol() {}
    Symbol(const std::string& s) : std::string(s) {}
    const std::string& text() const { return *this; }
    virtual const    Terminal* is_terminal()     const { return nullptr; }
                     Terminal* is_terminal()           { return const_cast<   Terminal*>(const_cast<const Symbol*>(this)->is_terminal()); }
    virtual const NonTerminal* is_non_terminal() const { return nullptr; }
                  NonTerminal* is_non_terminal()       { return const_cast<NonTerminal*>(const_cast<const Symbol*>(this)->is_non_terminal()); }
    bool operator<(const Symbol& s) const { return text() < s.text(); }
    friend std::ostream& operator<<(std::ostream& os, const Symbol& s) { return os << s.text(); }
};

//------------------------------------------------------------------------------

struct Terminal    : Symbol
{
    using Symbol::Symbol;
    Terminal(const std::string& s) : Symbol(s) {}
    const Terminal* is_terminal() const { return this; }
};

//------------------------------------------------------------------------------

struct NonTerminal : Symbol
{
    using Symbol::Symbol;
    NonTerminal(const std::string& s) : Symbol(s) {}
    const NonTerminal* is_non_terminal() const { return this; }
};

//------------------------------------------------------------------------------

typedef polymorphic<nonowning<NonTerminal>> non_terminal;
typedef polymorphic<nonowning<   Terminal>>     terminal;
typedef polymorphic<nonowning<Symbol>>            symbol;

//------------------------------------------------------------------------------

typedef std::set<non_terminal> non_terminal_set;
typedef std::set<    terminal>     terminal_set;
typedef std::vector<symbol> symbol_sequence;
typedef std::pair<const non_terminal, symbol_sequence> Production;

//------------------------------------------------------------------------------

inline std::ostream& operator<<(std::ostream& os, const Production& p)
{
    os << p.first << " \t= ";
    return separated_output(os, p.second, " ") << " ;";
}

//------------------------------------------------------------------------------

struct Grammar
{
    typedef std::multimap<non_terminal, symbol_sequence> productions_map;

    non_terminal start_symbol() const { return m_start_symbol; }
            void start_symbol(non_terminal s) { m_start_symbol = s; }

    non_terminal get_non_terminal(const char* name);
        terminal get_terminal(const char* name);
        terminal epsilon() { return get_terminal("\u03B5"); } // We use empty string as epsilon, don't return it as any other token
        terminal eof()     { return get_terminal("\u0024"); } // We use ETX - End of Text as EOF terminal, don't return it as token!

    std::set<non_terminal> nonterminals() const;

    const productions_map& productions() const { return m_productions; }
          productions_map& productions()       { return m_productions; }

    void append(Production&& p);

    friend std::ostream& operator<<(std::ostream& os, const Grammar& r)
    {
        for (const auto& p : r.productions()) os << p << std::endl;
        return os;
    }

private:

    typedef polymorphic<owning<NonTerminal>>       non_terminal_own;
    typedef polymorphic<owning<   Terminal>>           terminal_own;

    std::set<non_terminal_own> m_nonterminals;
    std::set<    terminal_own> m_terminals;
    productions_map            m_productions;
    non_terminal               m_start_symbol = non_terminal(nullptr);

};

//------------------------------------------------------------------------------

inline non_terminal Grammar::get_non_terminal(const char* name)
{
    auto x = m_nonterminals.insert(name);
    return x.first->pointer();
}

//------------------------------------------------------------------------------

inline     terminal Grammar::get_terminal(const char* name)
{
    auto x = m_terminals.insert(name);
    return x.first->pointer();
}

//------------------------------------------------------------------------------

inline std::set<non_terminal> Grammar::nonterminals() const
{
    std::set<non_terminal> result(m_nonterminals.begin(), m_nonterminals.end());
    return result;
}

//------------------------------------------------------------------------------

inline void Grammar::append(Production&& p)
{
    m_productions.emplace(std::move(p.first), std::move(p.second));
}

//------------------------------------------------------------------------------
