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
#include <rpp/disposables/composite_disposable.hpp>

#include <queue>
#include <mutex>

namespace rpp::operators::details
{
template<typename T>
struct emission
{
    template<typename TT>
    emission(TT&& item, schedulers::time_point time)
        : value{std::forward<TT>(item)} 
        , time_point{time}{}

    std::variant<T, std::exception_ptr, rpp::utils::none> value{};
    rpp::schedulers::time_point                           time_point{};
};

template<rpp::constraint::observer Observer, typename Worker>
struct delay_disposable final : public rpp::composite_disposable, public std::enable_shared_from_this<delay_disposable<Observer, Worker>> {
    using T = rpp::utils::extract_observer_type_t<Observer>;

    delay_disposable(Observer&& in_observer, Worker&& in_worker, rpp::schedulers::duration delay)
        : observer(std::move(in_observer))
        , worker{std::move(in_worker)}
        , delay{delay}
    {
        add(worker.get_disposable());
    }

    Observer                     observer;
    RPP_NO_UNIQUE_ADDRESS Worker worker;
    rpp::schedulers::duration    delay{};

    std::mutex              mutex{};
    std::queue<emission<T>> queue;
    bool                    is_active{};
};

template<rpp::constraint::observer Observer, typename Worker>
struct delay_disposable_wrapper
{
    std::shared_ptr<delay_disposable<Observer, Worker>> disposable{};

    bool is_disposed() const { return disposable->is_disposed(); }
    void on_error(const std::exception_ptr& err) const { disposable->observer.on_error(err); }
};

template<rpp::constraint::observer Observer, typename Worker>
struct delay_observer_strategy
{
    std::shared_ptr<delay_disposable<Observer, Worker>> disposable{};

    void set_upstream(const rpp::disposable_wrapper& d) const
    {
        disposable->add(d.get_original());
    }

    bool is_disposed() const
    {
        return disposable->is_disposed();
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
        if (const auto delay = emplace_safe(std::forward<TT>(value)))
        {
            disposable->worker.schedule(delay.value(), [](const delay_disposable_wrapper<Observer, Worker>& wrapper) -> schedulers::optional_duration { return drain_queue(wrapper.disposable); }, delay_disposable_wrapper<Observer, Worker>{disposable});
        }
    }

    template<typename TT>
    std::optional<rpp::schedulers::duration> emplace_safe(TT&& item) const
    {
        const auto delay = std::is_same_v<std::exception_ptr, std::decay_t<TT>> ? schedulers::duration{0} : disposable->delay;

        std::lock_guard lock{disposable->mutex};
        if (std::is_same_v<std::exception_ptr, std::decay_t<TT>>)
            disposable->queue = std::queue<emission<rpp::utils::extract_observer_type_t<Observer>>>{};

        disposable->queue.emplace(std::forward<TT>(item), rpp::schedulers::clock_type::now() + delay);
        if (!disposable->is_active)
        {
            disposable->is_active = true;
            return delay;
        }
        return std::nullopt;
    }

    static schedulers::optional_duration drain_queue(const std::shared_ptr<delay_disposable<Observer, Worker>>& disposable)
    {
        while (true)
        {
            std::unique_lock lock{disposable->mutex};
            if (disposable->queue.empty())
            {
                disposable->is_active = false;
                return std::nullopt;
            }

            auto& top = disposable->queue.front();
            const auto now = rpp::schedulers::clock_type::now();
            if (top.time_point > now)
                return top.time_point - now;

            auto item = std::move(top.value);
            disposable->queue.pop();
            lock.unlock();

            std::visit(rpp::utils::overloaded{[&](rpp::utils::extract_observer_type_t<Observer>&& v) { disposable->observer.on_next(std::move(v)); },
                                              [&](const std::exception_ptr& err)                     { disposable->observer.on_error(err); },
                                              [&](rpp::utils::none)                                  { disposable->observer.on_completed(); }},
                       std::move(item));
        }
    }
};

template<rpp::schedulers::constraint::scheduler Scheduler>
struct delay_t
{
    template<rpp::constraint::decayed_type T>
    using ResultValue = T;

    rpp::schedulers::duration       duration;
    RPP_NO_UNIQUE_ADDRESS Scheduler scheduler;

    template<rpp::constraint::decayed_type Type, rpp::constraint::observer Observer>
    auto lift(Observer&& observer) const
    {
        using worker_t = decltype(scheduler.create_worker());

        auto disposable = std::make_shared<delay_disposable<std::decay_t<Observer>, worker_t>>(std::forward<Observer>(observer), scheduler.create_worker(), duration);
        disposable->observer.set_upstream(rpp::disposable_wrapper::from_weak(disposable));
        return rpp::observer<rpp::utils::extract_observer_type_t<Observer>, delay_observer_strategy<std::decay_t<Observer>, worker_t>>{std::move(disposable)};
    }
};
}

namespace rpp::operators
{
/**
 * @brief Shift the emissions from an Observable forward in time by a particular amount.
 * @details The delay operator modifies its source Observable by pausing for a particular increment of time (that you specify) before emitting each of the source Observable’s items. This has the effect of shifting the entire sequence of items emitted by the Observable forward in time by that specified increment.
 *
 * @marble delay
 {
     source observable        : +-1-2-3-|
     operator "delay: --"     : +---1-2-3-|
 }
 *
 * @details Actually this operator just schedules emissions via provided scheduler with provided delay_duration.
 *
 * @param delay_duration is the delay duration for emitting items. Delay duration should be able to cast to rpp::schedulers::duration.
 * @param scheduler provides the threading model for delay. e.g. With a new thread scheduler, the observer sees the values in a new thread after a delay duration to the subscription.
 * @warning #include <rpp/operators/delay.hpp>
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
    return details::delay_t<std::decay_t<Scheduler>>{delay_duration, std::forward<Scheduler>(scheduler)};
}
} // namespace rpp::operators