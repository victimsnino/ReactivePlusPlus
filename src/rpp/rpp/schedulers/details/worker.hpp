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

#include <rpp/schedulers/fwd.hpp>
#include <rpp/disposables/disposable_wrapper.hpp>
#include <rpp/observers/fwd.hpp>
#include <rpp/utils/constraints.hpp>
#include <rpp/defs.hpp>

namespace rpp::schedulers
{
template<constraint::strategy Strategy>
class worker
{
public:
    template<typename...Args>
        requires (!rpp::constraint::variadic_decayed_same_as<worker<Strategy>, Args...> && rpp::constraint::is_constructible_from<Strategy, Args&&...>)
    explicit worker(Args&& ...args)
        : m_strategy(std::forward<Args>(args)...) {}

    worker(const worker&) = default;
    worker(worker&&) noexcept = default;

    template<rpp::constraint::observer TObs, typename...Args>
    rpp::disposable_wrapper schedule(constraint::schedulable_fn<TObs, Args...> auto&& fn, TObs&& obs, Args&&...args) const
    {
        return m_strategy.defer(std::forward<decltype(fn)>(fn), std::forward<TObs>(obs), std::forward<Args>(args)...);
    }

    template<rpp::constraint::observer TObs, typename...Args>
    rpp::disposable_wrapper schedule(const duration delay, constraint::schedulable_fn<TObs, Args...> auto&& fn, TObs&& obs, Args&&...args) const
    {
        return schedule(m_strategy.now() + delay, std::forward<decltype(fn)>(fn), std::forward<TObs>(obs), std::forward<Args>(args)...);
    }

    template<rpp::constraint::observer TObs, typename...Args>
    rpp::disposable_wrapper schedule(const time_point time_point, constraint::schedulable_fn<TObs, Args...> auto&& fn, TObs&& obs, Args&&...args) const
    {
        return m_strategy.defer_at(time_point, std::forward<decltype(fn)>(fn), std::forward<TObs>(obs), std::forward<Args>(args)...);
    }

    static time_point now() { return Strategy::now(); }

private:
    RPP_NO_UNIQUE_ADDRESS Strategy m_strategy;
};
}