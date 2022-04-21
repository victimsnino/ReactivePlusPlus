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
#include <utility>

namespace rpp::utils
{
template<typename ...Types>
struct empty_function_t
{
    void operator()(const Types&...) const noexcept {}
};

struct rethrow_error_t
{
    void operator()(const std::exception_ptr &err) const { std::rethrow_exception(err); }
};

template<typename Observer>
auto make_forwarding_on_next(const Observer& obs)
{
    return [obs](auto&& v){obs.on_next(std::forward<decltype(v)>(v));};
}

template<typename Observer>
auto make_forwarding_on_error(const Observer& obs)
{
    return [obs](const std::exception_ptr& err){obs.on_error(err);};
}

template<typename Observer>
auto make_forwarding_on_completed(const Observer& obs)
{
    return [obs](){obs.on_completed();};
}
} // namespace rpp::utils
