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

#include <rpp/observers/fwd.hpp> // own forwarding

#include <exception>

namespace rpp
{
namespace details
{
struct observer_tag {};
} // namespace details

/**
 * \tparam T is type of value handled by this observer
 */
template<constraint::decayed_type T>
struct interface_observer : public details::observer_tag{};

} // namespace rpp
