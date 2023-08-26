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
template<rpp::constraint::observer Observer, typename Worker>
struct delay_disposable final : public rpp::composite_disposable, public std::enable_shared_from_this<delay_disposable<Observer, Worker>> {
    using T = rpp::utils::extract_observer_type_t<Observer>;

    delay_disposable(Observer&& in_observer, Worker&& in_worker, rpp::schedulers::duration delay)
        : worker{std::move(in_worker)}
        , observer(std::move(in_observer))
        , delay{delay}
    {
        add(worker.get_disposable());
    }

    struct emission
    {
        std::variant<T, std::exception_ptr, rpp::utils::none> value{};
        rpp::schedulers::time_point                           time_point{};
    };

    Worker                    worker;
    Observer                  observer;
    rpp::schedulers::duration delay{};

    std::mutex           mutex{};
    std::queue<emission> queue;
    bool                 is_active{};
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
        if (const auto timepoint = emplace_safe(std::forward<TT>(value)))
        {
            disposable->worker.schedule(timepoint.value(), [*this]() -> schedulers::optional_duration { return drain_queue(); });
        }
    }

    template<typename TT>
    std::optional<rpp::schedulers::time_point> emplace_safe(TT&& item) const
    {
        const auto delay = std::is_same_v<std::exception_ptr, std::decay_t<TT>> ? schedulers::duration{0} : disposable->delay;

        std::lock_guard lock{disposable->mutex};
        const auto timepoint = disposable->worker.now() + delay;
        disposable->queue.emplace(std::forward<TT>(item), timepoint);
        if (!disposable->is_active && disposable->queue.size() == 1)
        {
            disposable->is_active = true;
            return timepoint;
        }
        return {};
    }

    schedulers::optional_duration drain_queue() const
    {
        while (true)
        {
            std::unique_lock lock{disposable->mutex};
            if (disposable->queue.empty())
            {
                disposable->is_active = false;
                return {};
            }

            auto& top = disposable->queue.top();
            const auto now = disposable->worker.now();
            if (top.time_point > now)
                return top.time - now;

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

    template<rpp::constraint::observer Observer, typename... Strategies>
    void subscribe(Observer&& observer, const observable_chain_strategy<Strategies...>& observable_strategy) const
    {
        using worker_t = decltype(scheduler.create_worker());

        auto disposable = std::make_shared<delay_disposable<std::decay_t<Observer>, worker_t>>(std::forward<Observer>(observer), scheduler.create_worker(), duration);
        disposable->observer.set_upstream(rpp::disposable_wrapper::from_weak(disposable));
        observable_strategy.subscribe(rpp::observer<rpp::utils::extract_observer_type_t<Observer>, delay_observer_strategy<std::decay_t<Observer>, worker_t>>{std::move(disposable)});
    }
};
}

namespace rpp::operators
{
template<rpp::schedulers::constraint::scheduler Scheduler>
auto delay(rpp::schedulers::duration duration, Scheduler&& scheduler)
{
    return details::delay_t<std::decay_t<Scheduler>>{duration, std::forward<Scheduler>(scheduler)};
}
} // namespace rpp::operators