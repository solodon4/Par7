#pragma once

#include <iostream>     // std::clog for bison printer
#include <string>
#include <iterator>     // std::ostream_iterator
#include <vector>       // std::vector
#include <algorithm>    // std::copy

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

//==============================================================================
// Forward declarations
//==============================================================================

// To disable Clang's deprecation warning
#define register

//------------------------------------------------------------------------------

struct Term : std::string
{
    using std::string::string;
    virtual ~Term() {}
    Term(const std::string& s) : std::string(s) {}
    const std::string& text() const { return *this; }
    friend std::ostream& operator<<(std::ostream& os, const Term& t) { return os << t.text(); }
};

//------------------------------------------------------------------------------

struct Terminal    : Term { using Term::Term;    Terminal(const std::string& s) : Term(s) {} };

//------------------------------------------------------------------------------

struct NonTerminal : Term { using Term::Term; NonTerminal(const std::string& s) : Term(s) {} };

//------------------------------------------------------------------------------

struct Def  : std::vector<std::unique_ptr<Term>>
{
    using std::vector<std::unique_ptr<Term>>::vector;

    void prepend(std::unique_ptr<Term>&& t) { prepend_to(static_cast<std::vector<std::unique_ptr<Term>>&>(*this), t); }

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
