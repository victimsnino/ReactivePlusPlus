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

#include <rpp/observables/fwd.hpp>
#include <rpp/observables/interface_observable.hpp>            // base_class
#include <rpp/utils/operator_declaration.hpp>                  // for header include
#include <rpp/subscribers/dynamic_subscriber.hpp>
#include <utility>

namespace rpp
{
/**
 * \brief observable specified with specific type of OnSubscribeFn. Used to store OnSubscribeFn function as is on stack (instead of allocating it somewhere).
 *
 * It has better performance comparing to rpp::dynamic_observable. Use it if possible. But it has worse usability due to OnSubscribeFn template parameter.
 * \tparam Type is type of value provided by this observable
 * \tparam OnSubscribeFn is type of function/functor/callable used during subscription on this observable
 * \ingroup observables
 */
template<constraint::decayed_type Type, constraint::on_subscribe_fn<Type> OnSubscribeFn>
class specific_observable : public interface_observable<Type, specific_observable<Type, OnSubscribeFn>>
{
public:
    specific_observable(OnSubscribeFn&& on_subscribe)
        : m_state{ std::move(on_subscribe) } {}

    specific_observable(const OnSubscribeFn& on_subscribe)
        : m_state{ on_subscribe } {}

    specific_observable(const specific_observable<Type, OnSubscribeFn>&)     = default;
    specific_observable(specific_observable<Type, OnSubscribeFn>&&) noexcept = default;

    /**
     * \brief Converts rpp::specific_observable to rpp::dynamic_observable via type-erasure mechanism.
     */
    template <typename...Args>
    [[nodiscard]] auto as_dynamic() const & requires details::is_header_included<details::dynamic_observable_tag, Args...> { return rpp::dynamic_observable<Type>{*this};            }
    template <typename...Args>
    [[nodiscard]] auto as_dynamic() && requires details::is_header_included<details::dynamic_observable_tag, Args...>    { return rpp::dynamic_observable<Type>{std::move(*this)}; }

    friend struct details::member_overload<Type, specific_observable<Type, OnSubscribeFn>, details::subscribe_tag>;

private:

    // used by rpp::details::member_overload<Type, specific_observable<Type, OnSubscribeFn>, rpp::details::subscribe_tag>;
    template<constraint::observer_of_type<Type> Obs>
    composite_subscription subscribe_impl(const specific_subscriber<Type, Obs>& subscriber) const
    {
        if (!subscriber.is_subscribed())
            return subscriber.get_subscription();

        try
        {
            m_state(subscriber);
        }
        catch (...)
        {
            if (subscriber.is_subscribed())
                subscriber.on_error(std::current_exception());
            else
                throw;
        }
        return subscriber.get_subscription();
    }

private:
    [[no_unique_address]] OnSubscribeFn m_state;
};

template<typename OnSub>
specific_observable(OnSub on_subscribe) -> specific_observable<utils::extract_subscriber_type_t<utils::function_argument_t<OnSub>>, OnSub>;
} // namespace rpp
