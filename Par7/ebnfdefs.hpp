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
};

//------------------------------------------------------------------------------

struct Production
{
    polymorphic<nonowning<NonTerminal>>       lhs;
    std::vector<polymorphic<nonowning<Term>>> rhs;

    Production(
        polymorphic<nonowning<NonTerminal>>&&       l,
        std::vector<polymorphic<nonowning<Term>>>&& r)
    :
        lhs(std::move(l)), rhs(std::move(r))
    {}
};

//------------------------------------------------------------------------------

struct Grammar
{
    NonTerminal* nonterminal(const char* name);
       Terminal*    terminal(const char* name);
    void append(Production&& p);

private:

    typedef std::multimap<polymorphic<nonowning<NonTerminal>>, Production> productions_map;

    std::set<polymorphic<owning<NonTerminal>>> nonterminals;
    std::set<polymorphic<owning<   Terminal>>>    terminals;
    productions_map                             productions;

};

inline NonTerminal* Grammar::nonterminal(const char* name)
{
    auto x = nonterminals.insert(name);
    return x.first->pointer();
}

inline Terminal*    Grammar::terminal(const char* name)
{
    auto x = terminals.insert(name);
    return x.first->pointer();
}

inline void Grammar::append(Production&& p)
{
    productions.insert(productions_map::value_type(p.lhs, std::move(p.rhs)));
}
