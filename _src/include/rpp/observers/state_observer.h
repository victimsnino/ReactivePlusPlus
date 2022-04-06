// MIT License
// 
// Copyright (c) 2022 Aleksey Loginov
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#pragma once

#include <rpp/observers/interface_observer.h>
#include <rpp/subscribers/constraints.h>
#include <rpp/utils/constraints.h>
#include <rpp/utils/function_traits.h>

#include <exception>

namespace rpp::details
{

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
         std::invocable<T, State>                  OnNext,
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
    State       m_state;
    OnNext      m_on_next;
    OnError     m_on_err;
    OnCompleted m_on_completed;
};

template<typename TState, typename TOnNext, typename ...Args>
state_observer(TState, TOnNext, Args...) -> state_observer<std::decay_t<utils::function_argument_t<TOnNext>>, TState, TOnNext, Args...>;
} // namespace rpp::details