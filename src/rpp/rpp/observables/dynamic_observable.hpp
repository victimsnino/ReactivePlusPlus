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

#include <rpp/observers/dynamic_observer.hpp>

#include <memory>
#include <utility>

namespace rpp::details::observables
{
template<typename T, typename Observable>
void forwarding_subscribe(const void* const ptr, dynamic_observer<T>&& obs)
{
    static_cast<const Observable*>(ptr)->subscribe(std::move(obs));
}

template<rpp::constraint::decayed_type Type>
class dynamic_strategy final
{
public:
    using value_type = Type;

    template<rpp::constraint::observable_strategy<Type> Strategy>
        requires (!rpp::constraint::decayed_same_as<Strategy, dynamic_strategy<Type>>)
    explicit dynamic_strategy(observable<Type, Strategy>&& obs)
        : m_forwarder{std::make_shared<observable<Type, Strategy>>(std::move(obs))}
        , m_vtable{vtable::template create<observable<Type, Strategy>>()}
    {
    }

    template<rpp::constraint::observable_strategy<Type> Strategy>
        requires (!rpp::constraint::decayed_same_as<Strategy, dynamic_strategy<Type>>)
    explicit dynamic_strategy(const observable<Type, Strategy>& obs)
        : m_forwarder{std::make_shared<observable<Type, Strategy>>(obs)}
        , m_vtable{vtable::template create<observable<Type, Strategy>>()}
    {
    }

    dynamic_strategy(const dynamic_strategy&)     = default;
    dynamic_strategy(dynamic_strategy&&) noexcept = default;

    template<rpp::constraint::observer_strategy<Type> ObserverStrategy>
    void subscribe(observer<Type, ObserverStrategy>&& observer) const
    {
        m_vtable->subscribe(m_forwarder.get(), std::move(observer).as_dynamic());
    }

private:
    struct vtable
    {
        void (*subscribe)(const void*, dynamic_observer<Type>&&){};

        template<rpp::constraint::observable Observable>
        static const vtable* create() noexcept
        {
            static vtable s_res{
                .subscribe = forwarding_subscribe<Type, Observable>};
            return &s_res;
        }
    };

private:
    std::shared_ptr<void> m_forwarder;
    const vtable*         m_vtable;
};
}

namespace rpp
{
template<constraint::decayed_type Type>
class dynamic_observable : public observable<Type, details::observables::dynamic_strategy<Type>> {
public:
    using observable<Type, details::observables::dynamic_strategy<Type>>::observable;
};
}