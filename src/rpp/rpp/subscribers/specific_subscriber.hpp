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

#include <rpp/observers/specific_observer.hpp>
#include <rpp/subscribers/details/subscriber_base.hpp>
#include <rpp/utils/function_traits.hpp>
#include <rpp/observers/constraints.hpp>

namespace rpp
{
/**
 * \brief specific version of subscriber which stores type of observer used inside to prevent extra allocations
 * \tparam Type type of values expected by this subscriber
 * \tparam Observer observer which was wrapped by this subscriber
 */
template<constraint::decayed_type Type, constraint::decayed_observer Observer>
class specific_subscriber : public details::subscriber_base<Type>
{
public:
    template<typename ...Types>
    specific_subscriber(Types&&...vals) requires std::constructible_from<Observer, Types...>
        : details::subscriber_base<Type>{  }
        , m_observer{ std::forward<Types>(vals)... } {}

    template<typename ...Types>
    specific_subscriber(composite_subscription sub, Types&&...vals) requires std::constructible_from<Observer, Types...>
        : details::subscriber_base<Type>{ std::move(sub)}
        , m_observer{ std::forward<Types>(vals)... } {}

    const Observer& get_observer() const
    {
        return m_observer;
    }

    auto as_dynamic() const & { return dynamic_subscriber<Type>{*this}; }
    auto as_dynamic() && { return dynamic_subscriber<Type>{this->get_subscription(), std::move(m_observer)}; }
protected:
    void on_next_impl(const Type& val) const final
    {
        m_observer.on_next(val);
    }

    void on_next_impl(Type&& val) const final
    {
        m_observer.on_next(std::move(val));
    }

    void on_error_impl(const std::exception_ptr& err) const final
    {
        m_observer.on_error(err);
    }

    void on_completed_impl() const final
    {
        m_observer.on_completed();
    }

private:
    Observer m_observer{};
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