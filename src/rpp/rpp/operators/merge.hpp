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

#include "rpp/disposables/base_disposable.hpp"
#include "rpp/utils/constraints.hpp"
#include <rpp/operators/fwd.hpp>
#include <rpp/defs.hpp>
#include <rpp/operators/details/strategy.hpp>
#include <atomic>
#include <cstddef>
#include <mutex>

namespace rpp::operators::details
{
template<typename Lock>
class merge_disposable final : public base_disposable
{
public:
    std::lock_guard<Lock> lock_guard() { return std::lock_guard<Lock>{m_lock}; }

    void increment_on_completed() { m_on_completed_needed.fetch_add(1, std::memory_order_relaxed); }
    bool decrement_on_completed() { return m_on_completed_needed.fetch_sub(1, std::memory_order::acq_rel) == 1; }

private:
    Lock               m_lock{};
    std::atomic_size_t m_on_completed_needed{};
};

struct merge_observer_inner_strategy
{
    std::shared_ptr<merge_disposable<std::mutex>> disposable;

    static constexpr empty_on_subscribe on_subscribe{};

    void set_upstream(const rpp::constraint::observer auto&, const rpp::disposable_wrapper& d) const
    {
        disposable->add(d.get_original());
    }

    bool is_disposed() const
    {
        return disposable->is_disposed();
    }

    template<typename T>
    void on_next(const rpp::constraint::observer auto& obs, T&& v) const
    {
        auto lock = disposable->lock_guard();
        obs.on_next(std::forward<T>(v));
    }

    void on_error(const rpp::constraint::observer auto & obs, const std::exception_ptr& err) const
    {
        disposable->dispose();

        auto lock = disposable->lock_guard();
        obs.on_error(err);
    }

    void on_completed(const rpp::constraint::observer auto& obs) const
    {
        if (disposable->decrement_on_completed())
        {
            disposable->dispose();

            auto lock = disposable->lock_guard();
            obs.on_completed();
        }
    }
};

template<rpp::constraint::observable InnerObservable>
struct merge_observer_strategy
{
    using Value = rpp::utils::extract_observable_type_t<InnerObservable>;

    std::shared_ptr<merge_disposable<std::mutex>> disposable = std::make_shared<merge_disposable<std::mutex>>();

    void on_subscribe(rpp::dynamic_observer<Value>& obs) const
    {
        disposable->increment_on_completed();
        obs.set_upstream(disposable_wrapper{disposable});
    }

    void set_upstream(const rpp::constraint::observer auto&, const rpp::disposable_wrapper& d) const
    {
        disposable->add(d.get_original());
    }

    bool is_disposed() const
    {
        return disposable->is_disposed();
    }

    template<typename T>
    void on_next(rpp::dynamic_observer<Value> obs, T&& v) const
    {
        disposable->increment_on_completed();
        std::forward<T>(v).subscribe(rpp::observer<Value, operator_strategy_base<Value, rpp::dynamic_observer<Value>, merge_observer_inner_strategy>>{std::move(obs), disposable});
    }

    void on_error(const rpp::constraint::observer auto & obs, const std::exception_ptr& err) const
    {
        disposable->dispose();

        auto lock = disposable->lock_guard();
        obs.on_error(err);
    }

    void on_completed(const rpp::constraint::observer auto& obs) const
    {
        if (disposable->decrement_on_completed())
        {
            disposable->dispose();

            auto lock = disposable->lock_guard();
            obs.on_completed();
        }
    }
};


template<rpp::constraint::observable Observable>
class merge_observable_strategy
{
    using InnerObservable = rpp::utils::extract_observable_type_t<Observable>;
    using Value = rpp::utils::extract_observable_type_t<InnerObservable>;
public:
    merge_observable_strategy(const Observable& observable)
        : m_observable{observable} {}

    merge_observable_strategy(Observable&& observable)
        : m_observable{std::move(observable)} {}

    template<rpp::constraint::observer_strategy<Value> ObserverStrategy>
    void subscribe(rpp::observer<Value, ObserverStrategy>&& obs) const
    {
        m_observable.subscribe(rpp::observer<InnerObservable, operator_strategy_base<InnerObservable, rpp::dynamic_observer<Value>, merge_observer_strategy<InnerObservable>>>{std::move(obs).as_dynamic()});
    }

private:
    RPP_NO_UNIQUE_ADDRESS Observable m_observable;
};

template<rpp::constraint::observable TObservable>
using merge_observable = rpp::observable<rpp::utils::extract_observable_type_t<rpp::utils::extract_observable_type_t<TObservable>>, merge_observable_strategy<TObservable>>;

struct merge_t
{
    template<rpp::constraint::observable TObservable>
        requires rpp::constraint::observable<rpp::utils::extract_observable_type_t<TObservable>>
    auto operator()(TObservable&& observable) const
    {
        return merge_observable<std::decay_t<TObservable>>{std::forward<TObservable>(observable)};
    }
};
}

namespace rpp::operators
{
inline auto merge()
{
    return details::merge_t{};
}
} // namespace rpp::operators