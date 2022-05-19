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

#include <rpp/observers/interface_observer.hpp>     // base class
#include <rpp/subscribers/constraints.hpp>          // default forwarding functors
#include <rpp/utils/function_traits.hpp>            // extract argument type
#include <rpp/subscribers/specific_subscriber.hpp>  // create proxy subscriber

#include <exception>

namespace rpp::details
{
struct forwarding_on_next
{
    template<constraint::subscriber TSub>
    void operator()(auto&& v, TSub&& sub) const { sub.on_next(std::forward<decltype(v)>(v)); }
};

struct forwarding_on_error
{
    template<constraint::subscriber TSub>
    void operator()(const std::exception_ptr& err, TSub&& sub) const {sub.on_error(err);}
};

struct forwarding_on_completed
{
    template<constraint::subscriber TSub>
    void operator()(TSub&& sub) const {sub.on_completed();}
};

/**
 * \brief Special type of specific_observer which has some state which this observer stores and pass to each callback. Used for storing subscriber without extra copies
 */
template<constraint::decayed_type T,
         constraint::decayed_type State,
         std::invocable<T, State>                  OnNext = forwarding_on_next,
         std::invocable<std::exception_ptr, State> OnError = forwarding_on_error,
         std::invocable<State>                     OnCompleted= forwarding_on_completed>
class state_observer final : public interface_observer<T>
{
public:
    state_observer(constraint::decayed_same_as<State> auto&&        state,
                   std::invocable<T, State> auto&&                  on_next,
                   std::invocable<std::exception_ptr, State> auto&& on_error,
                   std::invocable<State> auto&&                     on_completed)
        : m_state{std::forward<decltype(state)>(state)}
        , m_on_next{std::forward<decltype(on_next)>(on_next)}
        , m_on_err{std::forward<decltype(on_error)>(on_error)}
        , m_on_completed{std::forward<decltype(on_completed)>(on_completed)} {}

    state_observer(const state_observer<T, State, OnNext, OnError, OnCompleted>& other)     = default;
    state_observer(state_observer<T, State, OnNext, OnError, OnCompleted>&& other) noexcept = default;

    void on_next(const T& v) const override                     { m_on_next(v, m_state);             }
    void on_next(T&& v) const override                          { m_on_next(std::move(v), m_state);  }
    void on_error(const std::exception_ptr& err) const override { m_on_err(err, m_state);            }
    void on_completed() const override                          { m_on_completed(m_state);         }

private:
    State                             m_state;
    [[no_unique_address]] OnNext      m_on_next;
    [[no_unique_address]] OnError     m_on_err;
    [[no_unique_address]] OnCompleted m_on_completed;
};

template<typename TState, typename TOnNext, typename ...Args>
state_observer(TState, TOnNext, Args...) -> state_observer<std::decay_t<utils::function_argument_t<TOnNext>>, TState, TOnNext, Args...>;

template<constraint::decayed_type                            Type, 
         typename                                            State,
         std::invocable<Type, State>                         OnNext, 
         std::invocable<std::exception_ptr, State>           OnError, 
         std::invocable<State>                               OnCompleted, 
         constraint::decayed_same_as<composite_subscription> Subscription = composite_subscription>
auto create_subscriber_with_state_impl(State&&        state,
                                       OnNext&&       on_next,
                                       OnError&&      on_error,
                                       OnCompleted&&  on_completed,
                                       Subscription&& sub = composite_subscription{})
{
    return specific_subscriber<Type, state_observer<Type,
                                                    std::decay_t<State>,
                                                    std::decay_t<OnNext>,
                                                    std::decay_t<OnError>,
                                                    std::decay_t<OnCompleted>>>
    {
        std::forward<Subscription>(sub),
        std::forward<State>(state),
        std::forward<OnNext>(on_next),
        std::forward<OnError>(on_error),
        std::forward<OnCompleted>(on_completed)
    };
}

template<constraint::decayed_type                  Type, 
         typename                                  State,
         std::invocable<Type, State>               OnNext, 
         std::invocable<std::exception_ptr, State> OnError, 
         std::invocable<State>                     OnCompleted>
auto create_subscriber_with_state(State&&        state,
                                  OnNext&&      on_next,
                                  OnError&&     on_error,
                                  OnCompleted&& on_completed)
{
    return create_subscriber_with_state_impl<Type>(std::forward<State>(state),
                                                   std::forward<OnNext>(on_next),
                                                   std::forward<OnError>(on_error),
                                                   std::forward<OnCompleted>(on_completed));
}

template<constraint::decayed_type                            Type, 
         constraint::decayed_same_as<composite_subscription> Subscription,
         typename                                            State,
         std::invocable<Type, State>                         OnNext, 
         std::invocable<std::exception_ptr, State>           OnError, 
         std::invocable<State>                               OnCompleted>
auto create_subscriber_with_state(Subscription&& sub,
                                  State&&        state,
                                  OnNext&&       on_next,
                                  OnError&&      on_error,
                                  OnCompleted&&  on_completed)
{
    return create_subscriber_with_state_impl<Type>(std::forward<State>(state),
                                                   std::forward<OnNext>(on_next),
                                                   std::forward<OnError>(on_error),
                                                   std::forward<OnCompleted>(on_completed),
                                                   std::forward<Subscription>(sub));
}
} // namespace rpp::details