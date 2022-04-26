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

#include <rpp/observers/interface_observer.hpp>
#include <rpp/subscribers/constraints.hpp>
#include <rpp/utils/constraints.hpp>
#include <rpp/utils/function_traits.hpp>

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
} // namespace rpp::details