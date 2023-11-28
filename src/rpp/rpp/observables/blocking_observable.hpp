//                   ReactivePlusPlus library
//
//           Copyright Aleksey Loginov 2023 - present.
//  Distributed under the Boost Software License, Version 1.0.
//     (See accompanying file LICENSE_1_0.txt or copy at
//           https://www.boost.org/LICENSE_1_0.txt)
//
//  Project home: https://github.com/victimsnino/ReactivePlusPlus

#pragma once

#include <rpp/observables/fwd.hpp>

#include <rpp/defs.hpp>
#include <rpp/operators/details/strategy.hpp>

#include <condition_variable>
#include <mutex>

namespace rpp::details::observables
{
class blocking_disposble final : public base_disposable
{
public:
    void wait() 
    {
        std::unique_lock lock{m_mutex};
        m_cv.wait(lock, [this] { return is_disposed(); });
    }

    void dispose_impl() noexcept override 
    {
        m_cv.notify_all();
    }

private:
    std::mutex              m_mutex{};
    std::condition_variable m_cv{};
};

template<rpp::constraint::decayed_type Type, rpp::constraint::observable_strategy<Type> Strategy>
class blocking_strategy
{
public:
    using value_type = Type;
    using expected_disposable_strategy = typename rpp::details::observables::deduce_disposable_strategy_t<Strategy>::template add<1>;

    blocking_strategy(observable<Type, Strategy>&& observable)
        : m_original{std::move(observable)}
    {
    }

    blocking_strategy(const observable<Type, Strategy>& observable)
        : m_original{observable}
    {
    }

    template<rpp::constraint::observer_strategy<Type> ObserverStrategy>
    void subscribe(observer<Type, ObserverStrategy>&& obs) const
    {
        auto d = std::make_shared<blocking_disposble>();
        obs.set_upstream(d);
        m_original.subscribe(std::move(obs));
        
        if (!d->is_disposed())
            d->wait();
    }

private:
    RPP_NO_UNIQUE_ADDRESS observable<Type, Strategy> m_original;
};
}

namespace rpp
{
template<constraint::decayed_type Type, constraint::observable_strategy<Type> Strategy>
class blocking_observable : public observable<Type, details::observables::blocking_strategy<Type, Strategy>> {
public:
    using observable<Type, details::observables::blocking_strategy<Type, Strategy>>::observable;
};
}