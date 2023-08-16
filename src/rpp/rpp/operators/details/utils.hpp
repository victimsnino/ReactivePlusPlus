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

#include <optional>
#include <mutex>

namespace rpp::operators::details
{
template<typename T>
struct value_with_mutex
{
    std::optional<T> value{};
    std::mutex       mutex{};
};
} // namespace rpp::operators::details