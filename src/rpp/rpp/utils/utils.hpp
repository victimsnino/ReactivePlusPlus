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

#include <iterator>

namespace rpp::utils {

template<constraint::iterable T>
using iterable_value_t = std::iter_value_t<decltype(std::begin(std::declval<T>()))>;
}