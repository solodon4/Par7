#pragma once

#include <iostream>
#include <string>
#include <tuple>
#include <vector>

//==============================================================================
// Forward declarations
//==============================================================================

// To disable Clang's deprecation warning
#define register

struct Term : std::string { using std::string::string; virtual ~Term() {} Term(const std::string& s) : std::string(s) {} };

struct Terminal    : Term { using Term::Term;    Terminal(const std::string& s) : Term(s) {} };

struct NonTerminal : Term { using Term::Term; NonTerminal(const std::string& s) : Term(s) {} };

struct Def  : std::vector<std::unique_ptr<Term>> {};

struct Definitions : std::vector<Def> { using std::vector<Def>::vector; };

struct Rule : Definitions { std::string nonterminal; };

struct Syntax : std::vector<Rule> {};

