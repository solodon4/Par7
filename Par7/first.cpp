//
//  first.cpp
//  Par7
//
//  Created by Yuriy Solodkyy on 3/9/15.
//  Copyright (c) 2015 CrowdsTech. All rights reserved.
//

#include <set>
#include <string>
#include "ebnfdefs.hpp"

std::set<std::string> nonterminals(const Term& term)
{
    std::set<std::string> result;

    if (dynamic_cast<const NonTerminal*>(&term))
    {
        result.insert(term);
    }

    return result;
}

std::set<std::string> nonterminals(const Def& def)
{
    std::set<std::string> result;

    for (const auto& term : def)
    {
        std::set<std::string> tmp = nonterminals(*term);
        result.insert(tmp.begin(), tmp.end());
    }

    return result;
}

std::set<std::string> nonterminals(const Alternatives& alt)
{
    std::set<std::string> result;
    return result;
}

std::set<std::string> nonterminals(const Rule& rule)
{
    std::set<std::string> result = nonterminals(static_cast<const Alternatives&>(rule));
    result.insert(rule.nonterminal);
    return result;
}

std::set<std::string> nonterminals(const Syntax& grammar)
{
    std::set<std::string> result;

    for (const auto& rule : grammar)
    {
        std::set<std::string> tmp = nonterminals(rule);
        result.insert(tmp.begin(), tmp.end());
    }

    return result;
}
