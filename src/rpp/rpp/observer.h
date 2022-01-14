// MIT License
// 
// Copyright (c) 2021 Aleksey Loginov
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

#include <rpp/details/observer_state.h>
#include <rpp/utils/function_traits.h>
#include <rpp/utils/functors.h>

#include <utility>

namespace rpp
{
template<typename Type>
class observer final
{
public:
    template<typename OnNext = utils::empty_functor<Type>,
             typename OnError = utils::empty_functor<std::exception_ptr>,
             typename OnCompleted = utils::empty_functor<>,
             typename Enabled = std::enable_if_t<std::is_invocable_v<OnNext, Type> && 
                                                 std::is_invocable_v<OnError, std::exception_ptr> && 
                                                 std::is_invocable_v<OnCompleted>>>
    observer(OnNext&& on_next = {}, OnError&& on_error = {}, OnCompleted&& on_completed = {})
        : m_observer_state{std::forward<OnNext>(on_next),
                           std::forward<OnError>(on_error),
                           std::forward<OnCompleted>(on_completed)} {}

    template<typename OnNext,
             typename OnCompleted,
             typename Enabled = std::enable_if_t<std::is_invocable_v<OnNext, Type> &&
                                                 std::is_invocable_v<OnCompleted>>>
    observer(OnNext&& on_next, OnCompleted&& on_completed)
        : m_observer_state{std::forward<OnNext>(on_next),
                           utils::empty_functor<std::exception_ptr>{},
                           std::forward<OnCompleted>(on_completed)} {}

    observer(const observer&)     = default;
    observer(observer&&) noexcept = default;

    template<typename U, typename = std::enable_if_t<std::is_same_v<std::decay_t<U>, std::decay_t<Type>>>>
    void on_next(U&& val) const
    {
        m_observer_state.on_next(std::forward<U>(val));
    }

    void on_error(std::exception_ptr err) const
    {
        m_observer_state.on_error(err);
    }

    void on_completed() const
    {
        m_observer_state.on_completed();
    }

private:
    details::observer_state<Type> m_observer_state;
};

template<typename OnNext, typename ...Args>
observer(OnNext, Args...) -> observer<utils::function_argument_t<OnNext>>;
} // namespace rpp
