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

#include <rpp/fwd.h>
#include <rpp/observers/dynamic_observer.h>
#include <rpp/observers/interface_observer.h>
#include <rpp/utils/function_traits.h>
#include <rpp/utils/functors.h>
#include <rpp/utils/type_traits.h>

namespace rpp
{
template<typename T,
         typename OnNext = utils::empty_function_t<T>,
         typename OnError = utils::empty_function_t<std::exception_ptr>,
         typename OnCompleted = utils::empty_function_t<>>
class specific_observer : public interface_observer<T>
{
public:
    template<typename TOnNext      = utils::empty_function_t<T>,
             typename TOnError     = utils::empty_function_t<std::exception_ptr>,
             typename TOnCompleted = utils::empty_function_t<>,
             typename              = utils::enable_if_observer_constructible_t<T, TOnNext, TOnError, TOnCompleted>>
    specific_observer(TOnNext&& on_next = {}, TOnError&& on_error = {}, TOnCompleted&& on_completed = {})
        : m_on_next{std::forward<TOnNext>(on_next)}
        , m_on_err{std::forward<TOnError>(on_error)}
        , m_on_completed{std::forward<TOnCompleted>(on_completed)} {}

    template<typename TOnNext,
             typename TOnCompleted,
             typename = utils::enable_if_observer_constructible_t<T, TOnNext, utils::empty_function_t<std::exception_ptr>, TOnCompleted>>
    specific_observer(TOnNext&& on_next, TOnCompleted&& on_completed)
        : specific_observer{std::forward<TOnNext>(on_next),
                            utils::empty_function_t<std::exception_ptr>{},
                            std::forward<TOnCompleted>(on_completed)} {}

    specific_observer(const dynamic_observer<T>& obs)
        : m_on_next{utils::make_forwarding_on_next(obs)}
        , m_on_err{utils::make_forwarding_on_error(obs)}
        , m_on_completed{utils::make_forwarding_on_completed(obs)} {}

    specific_observer(const specific_observer<T, OnNext, OnError, OnCompleted>& other)     = default;
    specific_observer(specific_observer<T, OnNext, OnError, OnCompleted>&& other) noexcept = default;

    void on_next(const T& v) const override { m_on_next(v); }
    void on_next(T&& v) const override { m_on_next(std::move(v)); }
    void on_error(const std::exception_ptr& err) const override { m_on_err(err); }
    void on_completed() const override { m_on_completed(); }

    [[nodiscard]] dynamic_observer<T> as_dynamic() const & { return *this; }
    [[nodiscard]] dynamic_observer<T> as_dynamic() &&      { return std::move(*this); }

private:
    const OnNext      m_on_next;
    const OnError     m_on_err;
    const OnCompleted m_on_completed;
};

template<typename OnNext, typename ...Args, typename = std::enable_if_t<utils::is_callable_v<OnNext>>>
specific_observer(OnNext, Args...) -> specific_observer<std::decay_t<utils::function_argument_t<OnNext>>, OnNext, Args...>;

template<typename T>
specific_observer(
    const dynamic_observer<T>&)->specific_observer<T,
    std::invoke_result_t<decltype(utils::make_forwarding_on_next<dynamic_observer<T>>), dynamic_observer<T>>,
    std::invoke_result_t<decltype(utils::make_forwarding_on_error<dynamic_observer<T>>), dynamic_observer<T>>,
    std::invoke_result_t<decltype(utils::make_forwarding_on_completed<dynamic_observer<T>>), dynamic_observer<T>>>;
} // namespace rpp
