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

#include <iostream>     // std::clog for bison printer
#include <string>
#include <iterator>     // std::ostream_iterator
#include <set>          // std::set
#include <vector>       // std::vector
#include <algorithm>    // std::copy
#include <map>

//------------------------------------------------------------------------------

template <typename T, typename A, typename U>
void prepend_to(std::vector<T,A>& v, U&& u)
{
    if (v.empty())
        v.emplace_back(std::move(u));
    else
        v.emplace(v.begin(),std::move(u));
}

//------------------------------------------------------------------------------

template <typename T, typename A>
std::ostream& separated_output(std::ostream& os, const std::vector<T,A>& v, const char* separator)
{
    if (!v.empty())
    {
        std::ostream_iterator<T> out_it(os, separator);
        std::copy(v.begin(), v.end()-1, out_it);
        os << v.back();
    }

    return os;
}

//------------------------------------------------------------------------------

template <typename T, typename A>
std::ostream& separated_output(std::ostream& os, const std::set<T,A>& v, const char* separator)
{
    if (!v.empty())
    {
        std::ostream_iterator<T> out_it(os, separator);
        std::copy(v.begin(), v.end(), out_it);
    }

    return os;
}

//==============================================================================
// Forward declarations
//==============================================================================

// To disable Clang's deprecation warning
#define register

//------------------------------------------------------------------------------

struct Symbol : std::string
{
    using std::string::string;
    virtual ~Symbol() {}
    Symbol(const std::string& s) : std::string(s) {}
    const std::string& text() const { return *this; }
    bool operator<(const Symbol& s) const { return text() < s.text(); }
    friend std::ostream& operator<<(std::ostream& os, const Symbol& s) { return os << s.text(); }
};

//------------------------------------------------------------------------------

struct Terminal    : Symbol { using Symbol::Symbol;    Terminal(const std::string& s) : Symbol(s) {} };

//------------------------------------------------------------------------------

struct NonTerminal : Symbol { using Symbol::Symbol; NonTerminal(const std::string& s) : Symbol(s) {} };

//------------------------------------------------------------------------------

struct Def  : std::vector<std::unique_ptr<Symbol>>
{
    using std::vector<std::unique_ptr<Symbol>>::vector;

    void prepend(std::unique_ptr<Symbol>&& t) { prepend_to(static_cast<std::vector<std::unique_ptr<Symbol>>&>(*this), t); }

    friend std::ostream& operator<<(std::ostream& os, const Def& d)
    {
        for (const auto& p : d) os << ' ' << *p;
        return os;
    }
};

//------------------------------------------------------------------------------

struct Alternatives : std::vector<Def>
{
    using std::vector<Def>::vector;

    void prepend(Def&& d) { prepend_to(static_cast<std::vector<Def>&>(*this), d); }

    friend std::ostream& operator<<(std::ostream& os, const Alternatives& alts)
    {
        return separated_output(os, alts, " |");
    }
};

//------------------------------------------------------------------------------

struct Rule : Alternatives
{
    std::string nonterminal;
    Rule(std::string&& s, Alternatives&& a)
        : nonterminal(std::move(s))
        , Alternatives(std::move(a))
    {}

    friend std::ostream& operator<<(std::ostream& os, const Rule& r)
    {
        return os << r.nonterminal << " \t= " << (const Alternatives&)r << " ;";
    }
};

//------------------------------------------------------------------------------

struct Syntax : std::vector<Rule>
{
    using std::vector<Rule>::vector;

    void prepend(Rule&& r) { prepend_to(static_cast<std::vector<Rule>&>(*this), r); }

    friend std::ostream& operator<<(std::ostream& os, const Syntax& s)
    {
        return separated_output(os, s, "\n");
    }
};

//------------------------------------------------------------------------------

template <typename T>
class owning
{

    std::unique_ptr<T> ptr;

public:

    typedef T value_type;

    owning(owning&& v) : ptr(std::move(v.ptr)) {}

    template <typename... U>
    owning(U&&... u) : ptr(new T(std::forward<U>(u)...)) {}

    T* pointer() const { return ptr.get(); }
    
};

//------------------------------------------------------------------------------

template <typename T>
class nonowning
{

    T* ptr;

public:

    typedef T value_type;

    nonowning(const nonowning& v) : ptr(v.ptr) {}
    nonowning(nonowning&& v) : ptr(std::move(v.ptr)) {}

    nonowning(T*&& v) : ptr(std::move(v)) {}

    nonowning& operator=(const nonowning& v) { ptr = v.ptr; return *this; }

    T* pointer() const { return ptr; }

};

//------------------------------------------------------------------------------

template <typename Ptr>
struct polymorphic : Ptr
{
    using Ptr::Ptr;
    using typename Ptr::value_type;

    value_type& value() const { return *this->pointer(); }


    template <typename U>
    bool operator< (U&& u) const { return value() <  std::forward<U>(u); }
    bool operator< (const polymorphic& v) const { return value() < v.value(); }

    template <typename U>
    bool operator==(U&& u) const { return value() == std::forward<U>(u); }
    bool operator==(const polymorphic& v) const { return value() == v.value(); }

    friend std::ostream& operator<<(std::ostream& os, const polymorphic& p)
    {
        return os << *p.pointer();
    }
};

//------------------------------------------------------------------------------

typedef polymorphic<nonowning<NonTerminal>> non_terminal;
typedef polymorphic<nonowning<   Terminal>>     terminal;
typedef polymorphic<nonowning<Symbol>>            symbol;

//------------------------------------------------------------------------------

struct Production
{
    non_terminal        lhs;
    std::vector<symbol> rhs;

    Production(
        non_terminal&&        l,
        std::vector<symbol>&& r)
    :
        lhs(std::move(l)), rhs(std::move(r))
    {}

    friend std::ostream& operator<<(std::ostream& os, const Production& p)
    {
        os << p.lhs << " \t= ";
        return separated_output(os, p.rhs, " ") << " ;";
    }
};

//------------------------------------------------------------------------------

struct Grammar
{
    NonTerminal* nonterminal(const char* name);
       Terminal*    terminal(const char* name);
    void append(Production&& p);

    friend std::ostream& operator<<(std::ostream& os, const Grammar& r)
    {
        for (const auto& p : r.m_productions) os << p.second << std::endl;
        return os;
    }

private:

    typedef std::multimap<non_terminal, Production> productions_map;

    std::set<polymorphic<owning<NonTerminal>>> m_nonterminals;
    std::set<polymorphic<owning<   Terminal>>>    m_terminals;
    productions_map                             m_productions;

};

inline NonTerminal* Grammar::nonterminal(const char* name)
{
    auto x = m_nonterminals.insert(name);
    return x.first->pointer();
}

inline Terminal*    Grammar::terminal(const char* name)
{
    auto x = m_terminals.insert(name);
    return x.first->pointer();
}

inline void Grammar::append(Production&& p)
{
    m_productions.insert(productions_map::value_type(p.lhs, std::move(p)));
}
