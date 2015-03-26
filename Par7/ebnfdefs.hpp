#pragma once

#include <iostream>     // std::clog for bison printer
#include <string>
#include <iterator>     // std::ostream_iterator
#include <set>          // std::set
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

struct Term : std::string
{
    using std::string::string;
    virtual ~Term() {}
    Term(const std::string& s) : std::string(s) {}
    const std::string& text() const { return *this; }
    bool operator<(const Term& t) const { return text() < t.text(); }
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

template <typename T>
class polymorphic
{
private:

    std::unique_ptr<T> ptr;

public:

    polymorphic(polymorphic&& v) : ptr(std::move(v.ptr)) {}

    template <typename... U>
    polymorphic(U&&... u) : ptr(new T(std::forward<U>(u)...)) {}

    template <typename U>
    bool operator< (U&& u) const { return *ptr <  std::forward<U>(u); }
    bool operator< (const polymorphic& v) const { return *ptr < *v.ptr; }

    template <typename U>
    bool operator==(U&& u) const { return *ptr == std::forward<U>(u); }
    bool operator==(const polymorphic& v) const { return *ptr == *v.ptr; }

    T* pointer() const { return ptr.get(); }
};

//------------------------------------------------------------------------------

struct Production
{
    std::unique_ptr<NonTerminal>       lhs;
    std::vector<std::unique_ptr<Term>> rhs;
};

//------------------------------------------------------------------------------

struct Grammar
{
    NonTerminal* nonterminal(const char* name);
       Terminal*    terminal(const char* name);

    std::set<polymorphic<NonTerminal>> nonterminals;
    std::set<polymorphic<   Terminal>>    terminals;
};

inline NonTerminal* Grammar::nonterminal(const char* name)
{
    auto x = nonterminals.insert(polymorphic<NonTerminal>(name));
    return x.first->pointer();
}

inline Terminal*    Grammar::terminal(const char* name)
{
    auto x = terminals.insert(polymorphic<Terminal>(name));
    return x.first->pointer();
}
