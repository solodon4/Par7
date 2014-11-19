#pragma once

#include <iostream>
#include <string>

//==============================================================================
// Forward declarations
//==============================================================================
// Seems to be the only way to include definitions required by union in cou.tab.hpp

#include "ebnf.tab.hpp"

#include <string>
#include <tuple>
#include <vector>

struct Term : std::tuple<std::string> {};

struct Def  : std::vector<Term> {};

struct Definitions : std::vector<Def> {};

struct Rule : Definitions { std::string nonterminal; };

struct Syntax : std::vector<Rule> {};

