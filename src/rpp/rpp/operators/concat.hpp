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

#include "rpp/observables/fwd.hpp"
#include "rpp/utils/constraints.hpp"
#include <rpp/operators/fwd.hpp>
#include <rpp/defs.hpp>
#include <rpp/operators/details/strategy.hpp>
#include <rpp/disposables/base_disposable.hpp>
#include <rpp/disposables/disposable_wrapper.hpp>

#include <memory>
#include <type_traits>
#include <queue>

namespace rpp::operators::details
{
template<rpp::constraint::observable Observable>
struct concat_inner_strategy;

template<rpp::constraint::observable Observable>
class concat_disposable final : public base_disposable, public std::enable_shared_from_this<concat_disposable<Observable>>
{
public:
    template<rpp::constraint::decayed_same_as<Observable> TObservable>
    void on_new_observable(TObservable&& observable, const rpp::constraint::observer auto& obs)
    {
        if (m_inner_subscribed.exchange(true, std::memory_order::acq_rel))
        {
            std::lock_guard lock{m_queue_mutex};
            if (m_inner_subscribed.exchange(true, std::memory_order::relaxed))
            {
                m_observables.push(std::forward<TObservable>(observable));
                return;
            }
        }
        subscribe_inner_observer(observable, obs);
    }
    
    template<rpp::constraint::decayed_same_as<Observable> TObservable>
    void subscribe_inner_observer(const TObservable& observable, const rpp::constraint::observer auto& obs)
    {
        observable.subscribe(rpp::base_observer<rpp::utils::extract_observable_type_t<Observable>,
                                                operator_strategy_base<rpp::utils::extract_observable_type_t<Observable>, decltype(obs), concat_inner_strategy<Observable>>>{obs, this->shared_from_this()});
    }

private:
    void dispose_impl() override {}

private:
    std::mutex             m_queue_mutex{};
    std::queue<Observable> m_observables{};
    size_t                 m_on_completed_requires{1};
    std::atomic_bool       m_inner_subscribed{};
};

template<rpp::constraint::observable Observable>
struct concat_inner_strategy
{
    constexpr static forwarding_on_next_strategy on_next{};
    constexpr static forwarding_on_error_strategy on_error{};
    constexpr static forwarding_on_completed_strategy on_completed{};
    constexpr static forwarding_set_upstream_strategy set_upstream{};
    constexpr static forwarding_is_disposed_strategy is_disposed{};
};

template<rpp::constraint::observable Observable>
class concat_strategy
{
public:
    template<typename T>
    void on_next(const rpp::constraint::observer auto& obs, T&& v) const
    {
        // make_disposable_if_needed(obs);
    }

    constexpr static forwarding_on_error_strategy on_error{};

    void operator()(rpp::constraint::observer auto& observer, const rpp::disposable_wrapper& d) 
    {
        if (d.is_disposed())
            return;

        // make_disposable_if_needed(observer);
        m_disposable->add(d.get_original());
    }

    bool is_disposed() const { return m_disposable && m_disposable->is_disposed(); }

private:
    // void make_disposable_if_needed(rpp::constraint::observer auto& observer)
    // {
    //     if (!m_disposable)
    //     {
    //         m_disposable = std::make_shared<concat_disposable>();
    //         observer.set_upstream(disposable_wrapper{m_disposable});
    //     }
    // }

    std::shared_ptr<concat_disposable<Observable>> m_disposable{};
};

template<rpp::constraint::observable TObservable>
using concat_observable = dynamic_operator_observable<rpp::utils::extract_observable_type_t<TObservable>, std::decay_t<TObservable>, concat_strategy<std::decay_t<TObservable>>>;

struct concat_t
{
    template<rpp::constraint::observable TObservable>
        requires (rpp::constraint::observable<rpp::utils::extract_observable_type_t<TObservable>>)
    auto operator()(TObservable&& observable) const
    {
        return concat_observable<std::decay_t<TObservable>>{std::forward<TObservable>(observable)};
    }
};
} // namespace rpp::operators::details

namespace rpp::operators
{
inline auto concat()
{
    return details::concat_t{};
}
} // namespace rpp::operators