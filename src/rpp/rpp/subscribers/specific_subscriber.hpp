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

#include <rpp/defs.hpp>
#include <rpp/observers/constraints.hpp>
#include <rpp/observers/specific_observer.hpp>
#include <rpp/subscribers/fwd.hpp>
#include <rpp/subscribers/details/subscriber_base.hpp>
#include <rpp/subscriptions/composite_subscription.hpp>
#include <rpp/utils/function_traits.hpp>

namespace rpp
{
/**
 * \brief specific version of subscriber which stores type of observer used inside to prevent extra allocations
 * \tparam Type type of values expected by this subscriber
 * \tparam Observer observer which was wrapped by this subscriber
 */
template<constraint::decayed_type Type, constraint::decayed_observer Observer>
class specific_subscriber : public details::typed_subscriber_tag<Type>
                          , public details::subscriber_base
{
public:
    template<typename ...Types>
    specific_subscriber(Types&&...vals) requires std::constructible_from<Observer, Types...>
        : subscriber_base{}
        , m_observer{ std::forward<Types>(vals)... } {}

    template<typename ...Types>
    specific_subscriber(composite_subscription sub, Types&&...vals) requires std::constructible_from<Observer, Types...>
        : subscriber_base{ std::move(sub)}
        , m_observer{ std::forward<Types>(vals)... } {}

    const Observer& get_observer() const
    {
        return m_observer;
    }

    void on_next(const Type& val) const
    {
        on_next_impl(val);
    }

    void on_next(Type&& val) const
    {
        on_next_impl(std::move(val));
    }

    void on_error(const std::exception_ptr& err) const
    {
        do_if_subscribed_and_unsubscribe([&err, this] { m_observer.on_error(err); });
    }

    void on_completed() const
    {
        do_if_subscribed_and_unsubscribe([this] { m_observer.on_completed(); });
    }
    
    auto as_dynamic() const & { return dynamic_subscriber<Type>{*this}; }
    auto as_dynamic() && { return dynamic_subscriber<Type>{this->get_subscription(), std::move(m_observer)}; }

private:
    void on_next_impl(auto&& val) const
    {
        if (!is_subscribed()) [[unlikely]]
            return;

        try
        {
            m_observer.on_next(std::forward<decltype(val)>(val));
        }
        catch (...)
        {
            on_error(std::current_exception());
        }
    }

    RPP_NO_UNIQUE_ADDRESS Observer m_observer{};
};

template<constraint::observer TObs>
specific_subscriber(TObs observer) -> specific_subscriber<utils::extract_observer_type_t<TObs>, TObs>;

template<constraint::observer TObs>
specific_subscriber(composite_subscription, TObs observer) -> specific_subscriber<utils::extract_observer_type_t<TObs>, TObs>;

template<typename OnNext,
         typename ...Args,
         typename Type = std::decay_t<utils::function_argument_t<OnNext>>>
specific_subscriber(composite_subscription, OnNext, Args ...) -> specific_subscriber<Type, details::deduce_specific_observer_type_t<Type, OnNext, Args...>>;

template<typename OnNext,
         typename ...Args,
         typename Type = std::decay_t<utils::function_argument_t<OnNext>>>
specific_subscriber(OnNext, Args ...) -> specific_subscriber<Type, details::deduce_specific_observer_type_t<Type, OnNext, Args...>>;


/**
 * \brief Creation of rpp::specific_subscriber with manual providing of type of subscriber. In case of ability to determine type of subscriber by function -> use constructor
 */
template<typename Type, typename ...Args>
auto make_specific_subscriber(Args&& ...args) -> specific_subscriber<Type, details::deduce_specific_observer_type_t<Type, Args...>>
{
    return {std::forward<Args>(args)...};
}

template<typename Type, typename ...Args>
auto make_specific_subscriber(composite_subscription sub, Args&& ...args)  -> specific_subscriber<Type, details::deduce_specific_observer_type_t<Type, Args...>>
{
    return {std::move(sub), std::forward<Args>(args)...};
}

namespace constraint
{
    template<typename Type, typename...Args>
    concept specific_subscriber_constructible = requires(Args...args)
    {
        make_specific_subscriber<Type>(args...);
    };
}

} // namespace rpp