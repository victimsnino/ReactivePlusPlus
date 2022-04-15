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

#include <rpp/schedulers/fwd.h>

#include <concepts>

namespace rpp::schedulers::constraint
{
// returns nullopt in case of don't need to reshedule scheulable or some duration which will be added to "now" and resheduled
template<typename T>
concept schedulable_fn = std::is_invocable_r_v<optional_duration, T>;

template<typename T>
concept scheduler = std::is_base_of_v<details::scheduler_tag, std::decay_t<T>>;
} // namespace rpp::schedulers::constraint