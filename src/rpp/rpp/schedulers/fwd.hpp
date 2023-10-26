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

#include <chrono>
#include <optional>

namespace rpp::schedulers
{
using clock_type        = std::chrono::steady_clock;
using time_point        = clock_type::time_point;
using duration          = std::chrono::nanoseconds;
using optional_duration = std::optional<duration>;
}

namespace rpp::schedulers::details
{
struct fake_schedulable_handler
{
    constexpr static bool is_disposed() { return true; }

    static void on_error(const std::exception_ptr&) {}
};

struct none_disposable{};
}

namespace rpp::schedulers::constraint
{
// returns std::nullopt in case of don't need to re-schedule schedulable or some duration which will be added to "now" and re-scheduled
template<typename Fn, typename... Args>
concept schedulable_fn = std::is_invocable_r_v<optional_duration, Fn, Args&...> && std::same_as<optional_duration, std::invoke_result_t<Fn, Args&...>>;

template<typename Handler>
concept schedulable_handler = requires(const Handler& handler)
{
    {handler.is_disposed()} -> std::same_as<bool>;
    handler.on_error(std::exception_ptr{});
};

template<typename S>
concept strategy = requires(const S& s, const details::fake_schedulable_handler& handler)
{
    {s.defer_for(duration{}, std::declval<optional_duration(*)(const details::fake_schedulable_handler&)>(), handler)} -> std::same_as<void>;
    {s.get_disposable()} -> rpp::constraint::any_of<rpp::disposable_wrapper, details::none_disposable>;
    {S::now()} -> std::same_as<rpp::schedulers::time_point>;
};
}

namespace rpp::schedulers
{
template<rpp::schedulers::constraint::strategy Strategy>
class worker;

class immediate;
class current_thread;
class new_thread;

namespace defaults
{
    using iteration_scheduler = current_thread;
}
}

namespace rpp::schedulers::constraint
{
namespace details
{
    template<typename T>
    struct is_worker : std::false_type
    {
    };

    template<constraint::strategy Strategy>
    struct is_worker<rpp::schedulers::worker<Strategy>> : std::true_type
    {
    };
} // namespace details

template<typename W>
concept worker = details::is_worker<W>::value;

template<typename S>
concept scheduler = requires(const S& s)
{
    {s.create_worker()} -> worker;
};
}

namespace rpp::schedulers::utils
{
template<rpp::schedulers::constraint::scheduler Scheduler>
using get_worker_t = std::decay_t<decltype(std::declval<Scheduler>().create_worker())>;
}