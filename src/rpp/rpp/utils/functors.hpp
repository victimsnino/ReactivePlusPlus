//                  ReactivePlusPlus library
//
//          Copyright Aleksey Loginov 2022 - present.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)
//
// Project home: https://github.com/victimsnino/ReactivePlusPlus
//

#pragma once

#include <exception>

namespace rpp::utils
{
template<typename ...Types>
struct empty_function_t
{
    void operator()(const Types&...) const noexcept {}
};

struct rethrow_error_t
{
    [[noreturn]] void operator()(const std::exception_ptr &err) const { std::rethrow_exception(err); }
};
} // namespace rpp::utils