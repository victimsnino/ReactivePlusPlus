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

#include <rpp/observables/specific_observable.hpp>  // base
#include <rpp/observables/type_traits.hpp>          // extract_observable_type

#include <memory>

namespace rpp::details
{
template<constraint::decayed_type Type>
class dynamic_observable_state
{
public:
    template<constraint::observable_of_type<Type> TObs>
    dynamic_observable_state(TObs&& obs)
        : m_impl{std::make_shared<dynamic_observable_state_impl<std::decay_t<TObs>>>(std::forward<TObs>(obs))} {}

    composite_subscription operator()(const dynamic_subscriber<Type>& subscriber) const
    {
        return (*m_impl)(subscriber);
    }
private:
    struct interface_dynamic_observable_state_impl
    {
        virtual ~interface_dynamic_observable_state_impl() = default;

        virtual composite_subscription operator()(const dynamic_subscriber<Type>& subscriber) const = 0;
    };

    template<constraint::observable TObs>
    class dynamic_observable_state_impl : public interface_dynamic_observable_state_impl
    {
    public:
        dynamic_observable_state_impl(TObs&& observable)
            : m_observable{std::move(observable)} {}

        dynamic_observable_state_impl(const TObs& observable)
            : m_observable{observable} {}

        composite_subscription operator()(const dynamic_subscriber<Type>& subscriber) const override
        {
            return m_observable.subscribe(subscriber);
        }

    private:
        TObs m_observable{};
    };

    std::shared_ptr<interface_dynamic_observable_state_impl> m_impl{};
};
} // namespace rpp::details

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
class dynamic_observable : public specific_observable<Type, details::dynamic_observable_state<Type>>
{
public:
    using base = specific_observable<Type, details::dynamic_observable_state<Type>>;
    using base::base;

    dynamic_observable(constraint::on_subscribe_fn<Type> auto&& on_subscribe)
        : base{specific_observable<Type, std::decay_t<decltype(on_subscribe)>>(on_subscribe)} {}

    template<constraint::observable_of_type<Type> TObs>
        requires (!std::is_same_v<std::decay_t<TObs>, dynamic_observable<Type>>)
    dynamic_observable(TObs&& observable)
        : base{std::forward<TObs>(observable)} {}
};

template<constraint::observable TObs>
dynamic_observable(TObs obs) -> dynamic_observable<utils::extract_observable_type_t<TObs>>;

template<typename OnSub>
dynamic_observable(OnSub on_subscribe) -> dynamic_observable<utils::extract_subscriber_type_t<utils::function_argument_t<OnSub>>>;
} // namespace rpp
