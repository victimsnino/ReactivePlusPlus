//                  ReactivePlusPlus library
//
//          Copyright Aleksey Loginov 2023 - present.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)
//
// Project home: https://github.com/victimsnino/ReactivePlusPlus
//

#pragma once

#include <stdexcept>

namespace rpp::utils
{
template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

template<typename ...Types>
struct empty_function_t
{
    void operator()(const Types&...) const noexcept {}
};

struct empty_function_any_t
{
    template<typename ...Types>
    void operator()(const Types&...) const noexcept {}
};

struct rethrow_error_t
{
    [[noreturn]] void operator()(const std::exception_ptr& err) const noexcept { std::rethrow_exception(err); }
};

struct return_true
{
    bool operator()() const {return true;}
};
} // namespace rpp::utils