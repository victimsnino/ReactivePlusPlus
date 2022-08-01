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

#include <rpp/observers/constraints.hpp>    // wrapping constructor
#include <rpp/observers/state_observer.hpp> // base
#include <rpp/utils/function_traits.hpp>    // extract function args
#include <rpp/utils/functors.hpp>           // default arguments

#include <memory>

namespace rpp::details
{
template<constraint::decayed_type T>
struct dynamic_observer_state_base
{
    virtual ~dynamic_observer_state_base() = default;

    virtual void on_next(const T& v) const = 0;
    virtual void on_next(T&& v) const = 0;
    virtual void on_error(const std::exception_ptr& err) const = 0;
    virtual void on_completed() const = 0;

    struct forward_on_next
    {
        void operator()(auto&& v, const std::shared_ptr<dynamic_observer_state_base<T>>& state) const { state->on_next(std::forward<decltype(v)>(v)); }
    };

    struct forward_on_error
    {
        void operator()(const std::exception_ptr& err, const std::shared_ptr<dynamic_observer_state_base<T>>& state) const { state->on_error(err); }
    };

    struct forward_on_completed
    {
        void operator()(const std::shared_ptr<dynamic_observer_state_base<T>>& state) const  { state->on_completed(); }
    };
};

template<constraint::decayed_type T, constraint::observer_of_type<T> TObserver, typename ...Args>
std::shared_ptr<dynamic_observer_state_base<T>> make_dynamic_observer_state(Args&& ...args) requires std::constructible_from<std::decay_t<TObserver>, Args...>
{
    using DecayedObserver = std::decay_t<TObserver>;

    class dynamic_observer_state final : public dynamic_observer_state_base<T>
    {
    public:
        dynamic_observer_state(Args&& ...args)
            : m_observer{std::forward<Args>(args)...}{}

        void on_next(const T& v) const override { m_observer.on_next(v); }
        void on_next(T&& v) const override { m_observer.on_next(std::move(v)); }
        void on_error(const std::exception_ptr& err) const override { m_observer.on_error(err); }
        void on_completed() const override { m_observer.on_completed(); }

    private:
        DecayedObserver m_observer;
    };

    return std::make_shared<dynamic_observer_state>(std::forward<Args>(args)...);
}

template<constraint::decayed_type T, typename ...Args>
std::shared_ptr<dynamic_observer_state_base<T>> make_dynamic_observer_state_from_fns(Args&& ...args)
{
    return make_dynamic_observer_state<T, specific_observer<T,std::decay_t<Args>...>>(std::forward<Args>(args)...);
}
} // namespace rpp::details

namespace rpp
{
template<constraint::decayed_type T>
using base_for_dynamic_observer = details::state_observer<T,
                                                          typename details::dynamic_observer_state_base<T>::forward_on_next,
                                                          typename details::dynamic_observer_state_base<T>::forward_on_error,
                                                          typename details::dynamic_observer_state_base<T>::forward_on_completed,
                                                          std::shared_ptr<details::dynamic_observer_state_base<T>>>;
/**
 * \brief Dynamic (type-erased) version of observer (comparing to specific_observer)
 * \details It uses type-erasure mechanism to hide types of OnNext, OnError and OnCompleted callbacks. But it has higher cost in the terms of performance due to usage of heap.
 * Use it only when you need to store observer as member variable or something like this. In other cases prefer using "auto" to avoid converting to dynamic_observer
 * \tparam T is type of value handled by this observer
 * \ingroup observers
 */
template<constraint::decayed_type T>
class dynamic_observer final : public base_for_dynamic_observer<T>
{
public:
    template<constraint::on_next_fn<T>   OnNext      = utils::empty_function_t<T>,
             constraint::on_error_fn     OnError     = utils::rethrow_error_t,
             constraint::on_completed_fn OnCompleted = utils::empty_function_t<>>
    dynamic_observer(OnNext&& on_next = {}, OnError&& on_error = {}, OnCompleted&& on_completed = {})
        : dynamic_observer{details::make_dynamic_observer_state_from_fns<T>(std::forward<OnNext>(on_next),
                                                                            std::forward<OnError>(on_error),
                                                                            std::forward<OnCompleted>(on_completed))} { }

    dynamic_observer(constraint::on_next_fn<T> auto&& on_next, constraint::on_completed_fn auto&& on_completed)
        : dynamic_observer{std::forward<decltype(on_next)>(on_next),
                           utils::rethrow_error_t{},
                           std::forward<decltype(on_completed)>(on_completed)} {}

    template<constraint::observer_of_type<T> TObserver> 
        requires (!std::is_same_v<std::decay_t<TObserver>, dynamic_observer<T>>)
    dynamic_observer(TObserver&& obs)
        : dynamic_observer{details::make_dynamic_observer_state<T, std::decay_t<TObserver>>(std::forward<TObserver>(obs))} {}

    /**
     * \brief Do nothing for rpp::dynamic_observer. Created only for unification of interfaces with rpp::specific_observer
     */
    const dynamic_observer<T>& as_dynamic() const { return *this; }

private:
    dynamic_observer(std::shared_ptr<details::dynamic_observer_state_base<T>>&& state)
        : base_for_dynamic_observer<T>{typename details::dynamic_observer_state_base<T>::forward_on_next{},
                                       typename details::dynamic_observer_state_base<T>::forward_on_error{},
                                       typename details::dynamic_observer_state_base<T>::forward_on_completed{},
                                       std::move(state)} {}
};

template<constraint::observer TObserver>
dynamic_observer(TObserver) -> dynamic_observer<utils::extract_observer_type_t<TObserver>>;

template<typename OnNext, typename ...Args>
dynamic_observer(OnNext, Args...) -> dynamic_observer<utils::decayed_function_argument_t<OnNext>>;
} // namespace rpp
