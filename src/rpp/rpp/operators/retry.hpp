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

#include <rpp/defs.hpp>
#include <rpp/operators/details/strategy.hpp>

namespace rpp::operators::details
{
    template<rpp::constraint::observer TObserver, constraint::decayed_type Observable>
    struct retry_state_t final : public rpp::composite_disposable
    {
        retry_state_t(TObserver&& in_observer, const Observable& observable, size_t count)
            : observer(std::move(in_observer))
            , observable(observable)
            , count{count}
        {
        }

        RPP_NO_UNIQUE_ADDRESS TObserver  observer;
        RPP_NO_UNIQUE_ADDRESS Observable observable;
        size_t                           count;
        std::atomic<bool>                is_inside_drain{};
    };

    template<rpp::constraint::observer TObserver, typename TObservable>
    void drain(const std::shared_ptr<retry_state_t<TObserver, TObservable>>& state);

    template<rpp::constraint::observer TObserver, typename TObservable>
    struct retry_observer_strategy
    {
        using preferred_disposable_strategy = rpp::details::observers::none_disposable_strategy;

        std::shared_ptr<retry_state_t<TObserver, TObservable>> state;
        mutable bool                                           locally_disposed{};

        template<typename T>
        void on_next(T&& v) const
        {
            state->observer.on_next(std::forward<T>(v));
        }

        void on_error(const std::exception_ptr& err) const
        {
            locally_disposed = true;
            if (state->count == 0)
            {
                state->observer.on_error(err);
                state->dispose();
                return;
            }

            if (state->is_inside_drain.exchange(false, std::memory_order::seq_cst))
                return;

            drain(state);
        }

        void on_completed() const
        {
            locally_disposed = true;
            state->observer.on_completed();
        }

        void set_upstream(const disposable_wrapper& d) const
        {
            state->add(d);
        }

        bool is_disposed() const { return locally_disposed || state->is_disposed(); }
    };

    template<rpp::constraint::observer TObserver, typename TObservable>
    void drain(const std::shared_ptr<retry_state_t<TObserver, TObservable>>& state)
    {
        while (!state->is_disposed())
        {
            --state->count;
            state->clear();
            state->is_inside_drain.store(true, std::memory_order::seq_cst);
            try
            {
                using value_type = rpp::utils::extract_observer_type_t<TObserver>;
                state->observable.subscribe(observer<value_type, retry_observer_strategy<TObserver, TObservable>>{state});

                if (state->is_inside_drain.exchange(false, std::memory_order::seq_cst))
                    return;
            }
            catch (...)
            {
                state->observer.on_error(std::current_exception());
                return;
            }
        }
    }

    struct retry_t
    {
        const size_t count{};

        template<rpp::constraint::decayed_type T>
        struct operator_traits
        {
            using result_type = T;
        };

        template<rpp::details::observables::constraint::disposable_strategy Prev>
        using updated_disposable_strategy = rpp::details::observables::fixed_disposable_strategy_selector<1>;

        template<rpp::constraint::observer TObserver, typename TObservable>
        void subscribe(TObserver&& observer, TObservable&& observble) const
        {
            const auto d   = disposable_wrapper_impl<retry_state_t<std::decay_t<TObserver>, std::decay_t<TObservable>>>::make(std::forward<TObserver>(observer), std::forward<TObservable>(observble), count + 1);
            auto       ptr = d.lock();

            ptr->observer.set_upstream(d.as_weak());
            drain(ptr);
        }
    };
} // namespace rpp::operators::details

namespace rpp::operators
{
    /**
     * @brief If an error occurs, resubscribe to the same observable. Repeat it up to the specified count.
     *
     * @marble retry
       {
           source observable    : +-1-x
           operator "retry:(2)" : +-1-1-1-x
       }
     *
     * @param count is the number of retries
     *
     * @warning #include <rpp/operators/retry.hpp>
     *
     * @ingroup error_handling_operators
     * @see https://reactivex.io/documentation/operators/retry.html
     */
    inline auto retry(size_t count)
    {
        return details::retry_t{count};
    }
} // namespace rpp::operators
