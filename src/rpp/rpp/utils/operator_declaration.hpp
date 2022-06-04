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

#include <rpp/utils/constraints.hpp>

namespace rpp::details
{
template<class Tag, typename ...Args>
struct operator_declaration
{
    static std::false_type header_included();
};

template<typename ...Args>
concept is_header_included = decltype(operator_declaration<Args...>::header_included())::value;

#define IMPLEMENTATION_FILE(tag)                        \
template<typename ...Args>                              \
struct rpp::details::operator_declaration<rpp::details::tag, Args...> \
{                                                       \
    static std::true_type header_included();            \
}
} // namespace rpp::details
