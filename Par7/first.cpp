//
//  first.cpp
//  Par7
//
//  Created by Yuriy Solodkyy on 3/9/15.
//  Copyright (c) 2015 CrowdsTech. All rights reserved.
//

#include <set>
#include "ebnfdefs.hpp"

std::set<const NonTerminal*> nonterminals(const Syntax& grammar)
{
    std::set<const NonTerminal*> result;
    for (const auto& rule : grammar);
    return result;
}
