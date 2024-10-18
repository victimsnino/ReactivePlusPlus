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

#include <mutex>
#include <queue>

namespace rpp::operators::details
{
    template<typename T>
    struct emission
    {
        template<typename TT>
        emission(TT&& item, schedulers::time_point time)
            : value{std::forward<TT>(item)}
            , time_point{time}
        {
        }

        std::variant<T, std::exception_ptr, rpp::utils::none> value{};
        rpp::schedulers::time_point                           time_point{};
    };

    template<rpp::constraint::observer Observer, typename Worker>
    struct delay_state final
    {
        using T = rpp::utils::extract_observer_type_t<Observer>;

        delay_state(Observer&& in_observer, Worker&& in_worker, rpp::schedulers::duration delay)
            : observer(std::move(in_observer))
            , worker{std::move(in_worker)}
            , delay{delay}
        {
        }

        RPP_NO_UNIQUE_ADDRESS Observer observer;
        RPP_NO_UNIQUE_ADDRESS Worker   worker;
        rpp::schedulers::duration      delay;

        std::mutex              mutex{};
        std::queue<emission<T>> queue;
        bool                    is_active{};
    };

    template<rpp::constraint::observer Observer, typename Worker>
    struct delay_state_wrapper
    {
        std::shared_ptr<delay_state<Observer, Worker>> state{};

        bool is_disposed() const { return state->observer.is_disposed(); }

        void on_error(const std::exception_ptr& err) const { state->observer.on_error(err); }
    };

    template<rpp::constraint::observer Observer, typename Worker, bool ClearOnError>
    struct delay_observer_strategy
    {
        static constexpr auto preferred_disposable_mode = rpp::details::observers::disposable_mode::Auto;

        std::shared_ptr<delay_state<Observer, Worker>> state{};

        void set_upstream(const rpp::disposable_wrapper& d) const
        {
            state->observer.set_upstream(d);
        }

        bool is_disposed() const
        {
            return state->observer.is_disposed();
        }

        template<typename T>
        void on_next(T&& v) const
        {
            emplace(std::forward<T>(v));
        }

        void on_error(const std::exception_ptr& err) const noexcept
        {
            emplace(err);
        }

        void on_completed() const noexcept
        {
            emplace(rpp::utils::none{});
        }

    private:
        template<typename TT>
        void emplace(TT&& value) const
        {
            if (const auto tp = emplace_safe(std::forward<TT>(value)))
            {
                state->worker.schedule(
                    tp.value(),
                    [](const delay_state_wrapper<Observer, Worker>& wrapper) { return drain_queue(wrapper.state); },
                    delay_state_wrapper<Observer, Worker>{state});
            }
        }

        template<typename TT>
        std::optional<rpp::schedulers::time_point> emplace_safe(TT&& item) const
        {
            std::lock_guard lock{state->mutex};
            if constexpr (ClearOnError && rpp::constraint::decayed_same_as<std::exception_ptr, TT>)
            {
                state->queue = std::queue<emission<rpp::utils::extract_observer_type_t<Observer>>>{};
                state->observer.on_error(std::forward<TT>(item));
                return std::nullopt;
            }
            else
            {
                const auto tp = state->worker.now() + state->delay;
                state->queue.emplace(std::forward<TT>(item), tp);
                if (!state->is_active)
                {
                    state->is_active = true;
                    return tp;
                }
                return std::nullopt;
            }
        }

        static schedulers::optional_delay_to drain_queue(const std::shared_ptr<delay_state<Observer, Worker>>& state)
        {
            while (true)
            {
                std::unique_lock lock{state->mutex};
                if (state->queue.empty())
                {
                    state->is_active = false;
                    return std::nullopt;
                }

                auto& top = state->queue.front();
                if (top.time_point > state->worker.now())
                    return schedulers::optional_delay_to{top.time_point};

                auto item = std::move(top.value);
                state->queue.pop();
                lock.unlock();

                std::visit(rpp::utils::overloaded{[&](rpp::utils::extract_observer_type_t<Observer>&& v) { state->observer.on_next(std::move(v)); },
                                                  [&](const std::exception_ptr& err) { state->observer.on_error(err); },
                                                  [&](rpp::utils::none) {
                                                      state->observer.on_completed();
                                                  }},
                           std::move(item));
            }
        }
    };

    template<rpp::schedulers::constraint::scheduler Scheduler, bool ClearOnError>
    struct delay_t
    {
        template<rpp::constraint::decayed_type T>
        struct operator_traits
        {
            using result_type = T;
        };

        template<rpp::details::observables::constraint::disposable_strategy Prev>
        using updated_optimal_disposable_strategy = Prev;

        rpp::schedulers::duration       duration;
        RPP_NO_UNIQUE_ADDRESS Scheduler scheduler;

        template<rpp::constraint::decayed_type Type, rpp::constraint::observer Observer>
        auto lift(Observer&& observer) const
        {
            using worker_t = rpp::schedulers::utils::get_worker_t<Scheduler>;

            auto state = std::make_shared<delay_state<std::decay_t<Observer>, worker_t>>(std::forward<Observer>(observer), scheduler.create_worker(), duration);
            return rpp::observer<Type, delay_observer_strategy<std::decay_t<Observer>, worker_t, ClearOnError>>{std::move(state)};
        }
    };
} // namespace rpp::operators::details

namespace rpp::operators
{
    /**
     * @brief Shift the emissions from an Observable forward in time by a particular amount.
     * @details The delay operator modifies its source Observable by pausing for a particular increment of time (that you specify) before emitting each of the source Observable’s items. This has the effect of shifting the entire sequence of items emitted by the Observable forward in time by that specified increment.
     *
     * @marble delay
        {
            source observable      : +-1-2-3-#
            operator "delay:(--)"  : +---1-2-3-#
        }
     *
     * @details Actually this operator just schedules emissions via provided scheduler with provided delay_duration.
     * @warning on_error/on_completed invoking also would be delayed as any other emissions, so, WHOLE observable would be shifter. If you want to obtain `on_error` immediately, use `observe_on` instead.
     *
     * @param delay_duration is the delay duration for emitting items. Delay duration should be able to cast to rpp::schedulers::duration.
     * @param scheduler provides the threading model for delay. e.g. With a new thread scheduler, the observer sees the values in a new thread after a delay duration to the subscription.
     * @note `#include <rpp/operators/delay.hpp>`
     *
     * @par Examples
     * @snippet delay.cpp delay
     *
     * @ingroup utility_operators
     * @see https://reactivex.io/documentation/operators/delay.html
     */
    template<rpp::schedulers::constraint::scheduler Scheduler>
    auto delay(rpp::schedulers::duration delay_duration, Scheduler&& scheduler)
    {
        return details::delay_t<std::decay_t<Scheduler>, false>{delay_duration, std::forward<Scheduler>(scheduler)};
    }
} // namespace rpp::operators
