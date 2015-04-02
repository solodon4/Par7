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

//------------------------------------------------------------------------------

template <typename T, typename A>
std::set<T,A> difference(const std::set<T,A>& a, const std::set<T,A>& b)
{
    std::set<T,A> result;
    std::set_difference(a.begin(), a.end(), b.begin(), b.end(), std::inserter(result, result.begin()));
    return result;
}

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

    nonowning(T*   v) : ptr(v) {}
    //    nonowning(T*&& v) : ptr(std::move(v)) {}
    nonowning(const owning<T>& v) : ptr(v.pointer()) {}

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

    const value_type* operator&() const { return this->pointer(); }
    value_type* operator&()       { return this->pointer(); }

    friend std::ostream& operator<<(std::ostream& os, const polymorphic& p)
    {
        return os << *p.pointer();
    }
};

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
    typedef std::multimap<non_terminal, Production> productions_map;
    typedef polymorphic<owning<NonTerminal>>       non_terminal_own;
    typedef polymorphic<owning<   Terminal>>           terminal_own;

    NonTerminal* nonterminal(const char* name);
       Terminal*    terminal(const char* name);

    std::set<non_terminal> nonterminals() const;

    const productions_map& productions() const { return m_productions; }
          productions_map& productions()       { return m_productions; }

    void append(Production&& p);

    friend std::ostream& operator<<(std::ostream& os, const Grammar& r)
    {
        for (const auto& p : r.m_productions) os << p.second << std::endl;
        return os;
    }

private:

    std::set<non_terminal_own> m_nonterminals;
    std::set<    terminal_own> m_terminals;
    productions_map            m_productions;

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

inline std::set<non_terminal> Grammar::nonterminals() const
{
    std::set<non_terminal> result(m_nonterminals.begin(), m_nonterminals.end());
    return result;
}

inline void Grammar::append(Production&& p)
{
    m_productions.insert(productions_map::value_type(p.lhs, std::move(p)));
}
