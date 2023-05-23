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

#include <rpp/disposables/fwd.hpp>
#include <rpp/utils/constraints.hpp>
#include <rpp/observers/dynamic_observer.hpp>

#include <chrono>
#include <optional>
#include <concepts>
#include <type_traits>

namespace rpp::schedulers
{
using clock_type = std::chrono::steady_clock;
using time_point = clock_type::time_point;
using duration = std::chrono::nanoseconds;
using optional_duration = std::optional<duration>;
}

namespace rpp::schedulers::constraint
{
// returns std::nullopt in case of don't need to re-schedule schedulable or some duration which will be added to "now" and re-scheduled
template<typename Fn, typename...Args>
concept schedulable_fn = std::is_invocable_r_v<optional_duration, Fn, Args&...> && std::same_as<optional_duration, std::invoke_result_t<Fn, Args&...>>;

template<typename S>
concept strategy = requires(const S& s, const rpp::dynamic_observer<int>& obs)
{
    {s.defer_for(duration{}, std::declval<optional_duration(*)(const rpp::dynamic_observer<int>&)>(), obs)} -> std::same_as<void>;
    {s.get_disposable()} -> std::same_as<rpp::disposable_wrapper>;
};
}

namespace rpp::schedulers
{
template<rpp::schedulers::constraint::strategy Strategy>
class worker;

class immediate;
class current_thread;
class new_thread;
}

namespace rpp::schedulers::constraint
{
namespace details
{
    template<typename T>
    struct is_worker : std::false_type{};

    template<constraint::strategy Strategy>
    struct is_worker<rpp::schedulers::worker<Strategy>> : std::true_type{};
} // namespace details

template<typename W>
concept worker = details::is_worker<W>::value;

template<typename S>
concept scheduler = requires(const S& s)
{
    {s.create_worker()} -> worker;
};
}