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

#include <algorithm>    // std::copy
#include <iostream>     // std::ostream
#include <iterator>     // std::ostream_iterator
#include <map>          // std::map
#include <set>          // std::set
#include <vector>       // std::vector

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
std::ostream& separated_output(std::ostream& os, const std::set<T,A>& s, const char* separator)
{
    bool first = true;

    for (const auto& e : s)
    {
        if (first)
        {
            first = false;
        }
        else
        {
            os << separator;
        }

        os << e;
    }

    return os;
}

//------------------------------------------------------------------------------

template <typename T, typename A>
std::ostream& operator<<(std::ostream& os, const std::set<T,A>& v)
{
    os << '{';
    separated_output(os,v,",");
    return os << '}';
}

//------------------------------------------------------------------------------

template <typename K, typename T, typename A>
std::ostream& separated_output(std::ostream& os, const std::map<K,T,A>& v, const char* separator)
{
    if (!v.empty())
    {
        bool first = true;

        for (const auto& x : v)
        {
            os << (first ? "" : separator) << x.first << " \t-> " << x.second;
            first = false;
        }
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

template <typename T, typename A>
std::set<T,A> join(const std::set<T,A>& a, const std::set<T,A>& b)
{
    std::set<T,A> result;
    std::set_union(a.begin(), a.end(), b.begin(), b.end(), std::inserter(result, result.begin()));
    return result;
}

//------------------------------------------------------------------------------
