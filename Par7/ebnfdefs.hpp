#pragma once

#include <iostream>
#include <string>
#include <tuple>
#include <vector>

template <typename T, typename A, typename U>
void prepend_to(std::vector<T,A>& v, U&& u)
{
    if (v.empty())
        v.emplace_back(std::move(u));
    else
        v.emplace(v.begin(),std::move(u));
}

//==============================================================================
// Forward declarations
//==============================================================================

// To disable Clang's deprecation warning
#define register

struct Term : std::string
{
    using std::string::string;
    virtual ~Term() {}
    Term(const std::string& s) : std::string(s) {}
    const std::string& text() const { return *this; }
    friend std::ostream& operator<<(std::ostream& os, const Term& t) { return os << t.text(); }
};

struct Terminal    : Term { using Term::Term;    Terminal(const std::string& s) : Term(s) {} };

struct NonTerminal : Term { using Term::Term; NonTerminal(const std::string& s) : Term(s) {} };

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

struct Definitions : std::vector<Def>
{
    using std::vector<Def>::vector;

    void prepend(Def&& d) { prepend_to(static_cast<std::vector<Def>&>(*this), d); }

    friend std::ostream& operator<<(std::ostream& os, const Definitions& defs)
    {
        os << " | ";
        for (const auto& d : defs) os << d << '\n';
        return os;
    }
};

struct Rule : Definitions
{
    std::string nonterminal;
    Rule(std::string&& s, Definitions&& d)
        : nonterminal(std::move(s))
        , Definitions(std::move(d))
    {}

    friend std::ostream& operator<<(std::ostream& os, const Rule& r)
    {
        os << r.nonterminal << " -> ";
        for (const auto& a : r) os << a;
        return os;
    }
};

struct Syntax : std::vector<Rule>
{
    using std::vector<Rule>::vector;

    void prepend(Rule&& r) { prepend_to(static_cast<std::vector<Rule>&>(*this), r); }

    friend std::ostream& operator<<(std::ostream& os, const Syntax& s)
    {
        for (const auto& r : s) os << r << '\n';
        return os;
    }
};
