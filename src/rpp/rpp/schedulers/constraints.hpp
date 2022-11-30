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

#include <rpp/schedulers/fwd.hpp>
#include <rpp/subscriptions/fwd.hpp>

#include <concepts>

namespace rpp::schedulers::constraint
{
// returns std::nullopt in case of don't need to re-schedule schedulable or some duration which will be added to "now" and re-scheduled
template<typename T>
concept schedulable_fn = std::invocable<T> && std::same_as<std::invoke_result_t<T>, optional_duration>;

template<typename T>
concept inner_schedulable_fn = std::invocable<T> && std::same_as<std::invoke_result_t<T>, void>;

template<typename T>
concept worker = std::is_base_of_v<details::worker_tag, std::decay_t<T>>;

template<typename T>
concept scheduler = std::is_base_of_v<details::scheduler_tag, std::decay_t<T>> && requires(const T t)
{
    {t.create_worker(std::declval<rpp::composite_subscription>())} -> worker;
};

// Forbid trampoline for operator's schedulers! Reason: https://github.com/victimsnino/ReactivePlusPlus/issues/277#issuecomment-1332675622
template<typename T>
concept scheduler_not_trampoline = scheduler<T> && !std::derived_from<std::decay_t<T>, trampoline>;
} // namespace rpp::schedulers::constraint