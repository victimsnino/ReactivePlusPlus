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

#include <rpp/operators/fwd.hpp>

#include <rpp/operators/details/strategy.hpp>
#include <rpp/operators/details/utils.hpp>

#include <rpp/disposables/refcount_disposable.hpp>

#include <queue>
#include <cassert>


namespace rpp::operators::details
{
enum ConcatStage : uint8_t
{
    None        = 0,
    InsideDrain = 1,
    Active      = 2
};

template<rpp::constraint::observable TObservable, rpp::constraint::observer TObserver>
class concat_state_t final : public rpp::refcount_disposable
{
public:
    concat_state_t(TObserver&& observer)
        : m_observer{std::move(observer)}
    {
    }

    pointer_under_lock<TObserver> get_observer() { return pointer_under_lock{m_observer}; }
    pointer_under_lock<std::queue<TObservable>> get_queue() { return pointer_under_lock{m_queue}; }

    std::atomic<ConcatStage>& stage() { return m_stage; }

private:
    value_with_mutex<TObserver>               m_observer;
    value_with_mutex<std::queue<TObservable>> m_queue;
    std::atomic<ConcatStage>                  m_stage{};
};

template<rpp::constraint::observable TObservable, rpp::constraint::observer TObserver>
struct concat_observer_strategy_base
{
    concat_observer_strategy_base(std::shared_ptr<concat_state_t<TObservable, TObserver>> state)
        : state{std::move(state)}
    {
    }

    std::shared_ptr<concat_state_t<TObservable, TObserver>> state{};
    rpp::composite_disposable_wrapper          refcounted = state->add_ref();

    template<typename T>
    void on_next(T&& v) const { }

    void on_error(const std::exception_ptr& err) const
    {
        state->dispose();
        state->get_observer()->on_error(err);
    }

    void set_upstream(const disposable_wrapper& d) { refcounted.add(d); }

    bool is_disposed() const { return refcounted.is_disposed(); }
};

template<rpp::constraint::observable TObservable, rpp::constraint::observer TObserver>
struct concat_inner_observer_strategy : public concat_observer_strategy_base<TObservable, TObserver>
{
    using base = concat_observer_strategy_base<TObservable, TObserver>;

    using base::base;

    template<typename T>
    void on_next(T&& v) const
    {
        base::state->get_observer()->on_next(std::forward<T>(v));
    }

    void on_completed() const
    {
        base::refcounted.dispose();
        if (base::state->is_disposed())
            base::state->get_observer()->on_completed();

        ConcatStage current = base::state->stage().load(std::memory_order::seq_cst);
        if (current == ConcatStage::InsideDrain)
            return;

        assert(current == ConcatStage::Active);
        const auto queue = base::state->queue();

        if (base::state->stage().compare_exchange_strong(current, queue->empty() ? ConcatStage::None : ConcatStage::InsideDrain, std::memory_order::seq_cst)) 
        {
            if (queue->empty())
                return;
            // immediate drain with unlock of queue
        }
        
    }
};

template<rpp::constraint::observable TObservable, rpp::constraint::observer TObserver>
struct concat_observer_strategy : public concat_observer_strategy_base<TObservable, TObserver>
{
    using base = concat_observer_strategy_base<TObservable, TObserver>;
    using preferred_disposable_strategy = rpp::details::observers::none_disposable_strategy;

    concat_observer_strategy(TObserver&& observer)
        : base{std::make_shared<concat_state_t<TObservable, TObserver>>(std::move(observer))}
    {
        base::state->get_observer()->set_upstream(rpp::disposable_wrapper::from_weak(base::state));
    }

    template<typename T>
    void on_next(T&& v) const 
    { 
        ConcatStage current = ConcatStage::None;
        if (base::state->stage().compare_exchange_strong(current, ConcatStage::InsideDrain, std::memory_order::seq_cst)) 
        {
            // immediate drain
        } 
        else 
        {
            base::state->queue()->push(std::forward<T>(v));
        }
    }

    void on_completed() const
    {
        base::refcounted.dispose();
        if (base::state->is_disposed())
            base::state->get_observer()->on_completed();
    }
};

struct concat_t: public operators::details::template_operator_observable_strategy<concat_observer_strategy>
{
    template<rpp::constraint::decayed_type T>
        requires rpp::constraint::observable<T>
    using result_value = rpp::utils::extract_observable_type_t<T>;

    template<rpp::details::observables::constraint::disposable_strategy Prev>
    using updated_disposable_strategy = rpp::details::observables::fixed_disposable_strategy_selector<1>;
};
}


namespace rpp::operators
{
inline auto concat()
{
    return details::concat_t{};
}
}