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
#include <rpp/utils/operator_declaration.hpp>

namespace rpp::details
{
template<rpp::constraint::decayed_type Type, typename SpecificObservable, typename MemberTag>
struct member_overload;
} // namespace rpp::details
