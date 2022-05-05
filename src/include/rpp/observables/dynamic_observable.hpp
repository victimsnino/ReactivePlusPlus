//                  ReactivePlusPlus library
//
//          Copyright Aleksey Loginov 2022 - present.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)
//
// Project home: https://github.com/victimsnino/ReactivePlusPlus
//

#pragma once

#include <rpp/observables/interface_observable.hpp> // base class
#include <rpp/observables/specific_observable.hpp>  // m_observable
#include <rpp/observables/type_traits.hpp>          // extract_observable_type

#include <memory>

namespace rpp
{
/**
 * \brief type-erased alternative of observable (comparing to rpp::specific_observable).
 *
 * It uses type-erasure mechanism to hide type of OnSubscribeFn. But it has higher cost in the terms of performance due to usage of heap.
 * Use it only when you need to store observable as member variable or something like this
 * \tparam Type is type of value provided by this observable
 * \ingroup observables
 */
template<constraint::decayed_type Type>
class dynamic_observable : public interface_observable<Type, dynamic_observable<Type>>
{
public:
    dynamic_observable(constraint::on_subscribe_fn<Type> auto&& on_subscribe)
        : m_observable{std::make_shared<specific_observable<Type, std::decay_t<decltype(on_subscribe)>>>(on_subscribe)} {}

    template<constraint::observable_of_type<Type> TObs>
        requires (!std::is_same_v<std::decay_t<TObs>, dynamic_observable<Type>>)
    dynamic_observable(TObs&& observable)
        : m_observable{ std::make_shared<std::decay_t<TObs>>(std::forward<TObs>(observable)) } {}

    dynamic_observable(const dynamic_observable<Type>&)     = default;
    dynamic_observable(dynamic_observable<Type>&&) noexcept = default;

    composite_subscription subscribe(const dynamic_subscriber<Type>& subscriber) const final
    {
        return m_observable->subscribe(subscriber);
    }

    template<typename ...Args>
        requires (std::is_constructible_v<dynamic_subscriber<Type>, Args...> && !constraint::variadic_is_same_type<dynamic_subscriber<Type>, Args...>)
    composite_subscription subscribe(Args&&...args) const
    {
        return m_observable->subscribe(dynamic_subscriber<Type>{std::forward<Args>(args)...});
    }

    const dynamic_observable<Type>& as_dynamic() const { return *this; }

private:
    std::shared_ptr<virtual_observable<Type>> m_observable{};
};

template<constraint::observable TObs>
dynamic_observable(TObs obs) -> dynamic_observable<utils::extract_observable_type_t<TObs>>;

template<typename OnSub>
dynamic_observable(OnSub on_subscribe) -> dynamic_observable<utils::extract_subscriber_type_t<utils::function_argument_t<OnSub>>>;
} // namespace rpp
