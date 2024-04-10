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

#include <rpp/disposables/refcount_disposable.hpp>
#include <rpp/operators/details/strategy.hpp>
#include <rpp/utils/utils.hpp>

#include <cassert>
#include <queue>


namespace rpp::operators::details
{
    template<rpp::constraint::observable TObservable, rpp::constraint::observer TObserver>
    struct concat_inner_observer_strategy;

    enum ConcatStage : uint8_t
    {
        None                   = 0,
        Draining               = 1,
        CompletedWhileDraining = 2,
        Processing             = 3,
    };

    template<rpp::constraint::observable TObservable, rpp::constraint::observer TObserver>
    class concat_state_t final : public rpp::refcount_disposable
    {
    public:
        concat_state_t(TObserver&& observer)
            : m_observer{std::move(observer)}
        {
        }

        rpp::utils::pointer_under_lock<TObserver>               get_observer() { return m_observer; }
        rpp::utils::pointer_under_lock<std::queue<TObservable>> get_queue() { return m_queue; }

        std::atomic<ConcatStage>& stage() { return m_stage; }

        void drain(rpp::composite_disposable_wrapper refcounted)
        {
            while (!is_disposed())
            {
                const auto observable = get_observable();
                if (!observable)
                {
                    stage().store(ConcatStage::None, std::memory_order::relaxed);
                    refcounted.dispose();
                    if (is_disposed())
                        get_observer()->on_completed();
                    return;
                }

                if (handle_observable_impl(observable.value(), refcounted))
                    return;
            }
        }

        void handle_observable(const rpp::constraint::decayed_same_as<TObservable> auto& observable, rpp::composite_disposable_wrapper refcounted)
        {
            if (handle_observable_impl(observable, refcounted))
                return;

            drain(refcounted);
        }

    private:
        bool handle_observable_impl(const rpp::constraint::decayed_same_as<TObservable> auto& observable, rpp::composite_disposable_wrapper refcounted)
        {
            stage().store(ConcatStage::Draining, std::memory_order::relaxed);
            refcounted.clear();
            observable.subscribe(concat_inner_observer_strategy<TObservable, TObserver>{disposable_wrapper_impl<concat_state_t>{wrapper_from_this()}.lock(), std::move(refcounted)});

            ConcatStage current = ConcatStage::Draining;
            return stage().compare_exchange_strong(current, ConcatStage::Processing, std::memory_order::seq_cst);
        }

    private:
        std::optional<TObservable> get_observable()
        {
            auto queue = get_queue();
            if (queue->empty())
                return std::nullopt;
            auto observable = queue->front();
            queue->pop();
            return observable;
        }

    private:
        rpp::utils::value_with_mutex<TObserver>               m_observer;
        rpp::utils::value_with_mutex<std::queue<TObservable>> m_queue;
        std::atomic<ConcatStage>                              m_stage{};
    };

    template<rpp::constraint::observable TObservable, rpp::constraint::observer TObserver>
    struct concat_observer_strategy_base
    {
        concat_observer_strategy_base(std::shared_ptr<concat_state_t<TObservable, TObserver>> state, rpp::composite_disposable_wrapper refcounted)
            : state{std::move(state)}
            , refcounted{std::move(refcounted)}
        {
        }

        concat_observer_strategy_base(std::shared_ptr<concat_state_t<TObservable, TObserver>> state)
            : concat_observer_strategy_base{state, state->add_ref()}
        {
        }

        std::shared_ptr<concat_state_t<TObservable, TObserver>> state;
        rpp::composite_disposable_wrapper                       refcounted;

        void on_error(const std::exception_ptr& err) const
        {
            state->dispose();
            state->get_observer()->on_error(err);
        }

        void set_upstream(const disposable_wrapper& d) const { refcounted.add(d); }

        bool is_disposed() const { return refcounted.is_disposed(); }
    };

    template<rpp::constraint::observable TObservable, rpp::constraint::observer TObserver>
    struct concat_inner_observer_strategy : public concat_observer_strategy_base<TObservable, TObserver>
    {
        using base = concat_observer_strategy_base<TObservable, TObserver>;

        using base::concat_observer_strategy_base;

        template<typename T>
        void on_next(T&& v) const
        {
            base::state->get_observer()->on_next(std::forward<T>(v));
        }

        void on_completed() const
        {
            ConcatStage current{ConcatStage::Draining};
            if (base::state->stage().compare_exchange_strong(current, ConcatStage::CompletedWhileDraining, std::memory_order::seq_cst))
                return;

            assert(current == ConcatStage::Processing);

            base::state->drain(base::refcounted);
        }
    };

    template<rpp::constraint::observable TObservable, rpp::constraint::observer TObserver>
    struct concat_observer_strategy : public concat_observer_strategy_base<TObservable, TObserver>
    {
        using base                          = concat_observer_strategy_base<TObservable, TObserver>;
        using preferred_disposable_strategy = rpp::details::observers::none_disposable_strategy;

        concat_observer_strategy(TObserver&& observer)
            : base{init_state(std::move(observer))}
        {
        }

        template<typename T>
        void on_next(T&& v) const
        {
            ConcatStage current = ConcatStage::None;
            if (base::state->stage().compare_exchange_strong(current, ConcatStage::Draining, std::memory_order::seq_cst))
                base::state->handle_observable(std::forward<T>(v), base::state->add_ref());
            else
                base::state->get_queue()->push(std::forward<T>(v));
        }

        void on_completed() const
        {
            base::refcounted.dispose();
            if (base::state->is_disposed())
                base::state->get_observer()->on_completed();
        }


    private:
        static std::shared_ptr<concat_state_t<TObservable, TObserver>> init_state(TObserver&& observer)
        {
            const auto d   = disposable_wrapper_impl<concat_state_t<TObservable, TObserver>>::make(std::move(observer));
            auto       ptr = d.lock();
            ptr->get_observer()->set_upstream(d.as_weak());
            return ptr;
        }
    };

    struct concat_t : lift_operator<concat_t>
    {
        using lift_operator<concat_t>::lift_operator;

        template<rpp::constraint::decayed_type T>
        struct operator_traits
        {
            static_assert(rpp::constraint::observable<T>, "T is not observable");

            using result_type = rpp::utils::extract_observable_type_t<T>;

            template<rpp::constraint::observer_of_type<result_type> TObserver>
            using observer_strategy = concat_observer_strategy<T, TObserver>;
        };

        template<rpp::details::observables::constraint::disposable_strategy Prev>
        using updated_disposable_strategy = rpp::details::observables::fixed_disposable_strategy_selector<1>;
    };
} // namespace rpp::operators::details


namespace rpp::operators
{
    /**
     * @brief Make observable which would merge emissions from underlying observables but without overlapping (current observable completes THEN next started to emit its values)
     *
     * @marble concat
     {
         source observable :
         {
             +--1-2-3-|
             .....+4--6-|
         }
         operator "concat" : +--1-2-3-4--6-|
     }
     *
     * @details Actually it subscribes on first observable from emissions. When first observable completes, then it subscribes on second observable from emissions and etc...
     *
     * @tparam MemoryModel rpp::memory_model strategy used to handle provided observables
     *
     * @warning #include <rpp/operators/concat.hpp>
     *
     * @par Example
     * @snippet concat.cpp concat_as_operator
     *
     * @ingroup creational_operators
     * @see https://reactivex.io/documentation/operators/concat.html
     */
    inline auto concat()
    {
        return details::concat_t{};
    }
} // namespace rpp::operators
