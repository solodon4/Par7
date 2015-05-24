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
/// Pointer-like abstractions.
///
/// \author Yuriy Solodkyy <yuriy.solodkyy@gmail.com>
///
/// \see https://github.com/solodon4/Par7
/// \see https://github.com/solodon4/SELL
///

#pragma once

#include <iosfwd>     // std::ostream
#include <memory>     // std::unique_ptr

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
    using Ptr::Ptr; // Forward all construction calls to the actual pointer implementation
    using typename Ptr::value_type;

    value_type& value() const { return *this->pointer(); }

    explicit operator bool() const { return this->pointer() != nullptr; } // NOTE: Should this forward to value when operator bool is present there?

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

//------------------------------------------------------------------------------
