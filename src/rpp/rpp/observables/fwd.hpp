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

namespace rpp
{
template<constraint::decayed_type Type>
struct interface_observable;
} // namespace rpp

namespace rpp::constraint
{
template<typename T, typename Type> concept observable_of_type = std::derived_from<T, interface_observable<Type>>;
} // namespace rpp::constraint
