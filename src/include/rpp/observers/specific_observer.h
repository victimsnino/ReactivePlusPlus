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

#include <concepts>

namespace rpp
{
/**
 * \brief Observer specific with types of callbacks to avoid extra heap usage.
 *
 * It has better performance comparing to rpp::dynamic_observer. Use it if possible. But it has worse usability due to OnNext/OnError/OnCompleted template parameters.
 * \tparam T is type of value provided by this observable
 * \tparam OnNext type of on_next callback
 * \tparam OnError type of on_error callback
 * \tparam OnCompleted type of on_completed callback
 */
template<typename T,
       details::on_next_fn<T>   OnNext      = utils::empty_function_t<T>,
       details::on_error_fn     OnError     = utils::empty_function_t<std::exception_ptr>,
       details::on_completed_fn OnCompleted = utils::empty_function_t<>>
class specific_observer final : public interface_observer<T>
{
public:
     template<details::on_next_fn<T>   TOnNext      = utils::empty_function_t<T>,
              details::on_error_fn     TOnError     = utils::empty_function_t<std::exception_ptr>,
              details::on_completed_fn TOnCompleted = utils::empty_function_t<>>
    specific_observer(TOnNext&& on_next = {}, TOnError&& on_error = {}, TOnCompleted&& on_completed = {})
        : m_on_next{std::forward<TOnNext>(on_next)}
        , m_on_err{std::forward<TOnError>(on_error)}
        , m_on_completed{std::forward<TOnCompleted>(on_completed)} {}

     specific_observer(details::on_next_fn<T> auto&& on_next, details::on_completed_fn auto&& on_completed)
         : specific_observer{std::forward<decltype(on_next)>(on_next),
                             utils::empty_function_t<std::exception_ptr>{},
                             std::forward<decltype(on_completed)>(on_completed)} {}

    specific_observer(const dynamic_observer<T>& obs) 
        : m_on_next{utils::make_forwarding_on_next(obs)}
        , m_on_err{utils::make_forwarding_on_error(obs)}
        , m_on_completed{utils::make_forwarding_on_completed(obs)} {}

    specific_observer(const specific_observer<T, OnNext, OnError, OnCompleted>& other)     = default;
    specific_observer(specific_observer<T, OnNext, OnError, OnCompleted>&& other) noexcept = default;

    void on_next(const T& v) const override                     { m_on_next(v);             }
    void on_next(T&& v) const override                          { m_on_next(std::move(v));  }
    void on_error(const std::exception_ptr& err) const override { m_on_err(err);            }
    void on_completed() const override                          { m_on_completed();         }

    [[nodiscard]] dynamic_observer<T> as_dynamic() const & { return *this; }
    [[nodiscard]] dynamic_observer<T> as_dynamic() &&      { return std::move(*this); }

private:
    OnNext      m_on_next;
    OnError     m_on_err;
    OnCompleted m_on_completed;
};

template<typename OnNext>
specific_observer(OnNext) -> specific_observer<std::decay_t<utils::function_argument_t<OnNext>>, OnNext>;

template<typename OnNext, typename OnError, typename ...Args, typename = std::enable_if_t<std::is_invocable_v<OnError, std::exception_ptr>>>
specific_observer(OnNext, OnError, Args...) -> specific_observer<std::decay_t<utils::function_argument_t<OnNext>>, OnNext, OnError, Args...>;

template<typename OnNext, typename OnCompleted, typename = std::enable_if_t<std::is_invocable_v<OnCompleted>>>
specific_observer(OnNext, OnCompleted) -> specific_observer<std::decay_t<utils::function_argument_t<OnNext>>, OnNext, utils::empty_function_t<std::exception_ptr>, OnCompleted>;

template<typename T>
specific_observer(const dynamic_observer<T>&)->specific_observer<T,
                                                                 decltype(utils::make_forwarding_on_next(std::declval<dynamic_observer<T>>())),
                                                                 decltype(utils::make_forwarding_on_error(std::declval<dynamic_observer<T>>())),
                                                                 decltype(utils::make_forwarding_on_completed(std::declval<dynamic_observer<T>>()))>;

namespace details
{
    template<typename ...Args>
    auto deduce_specific_observer_type(const Args&...vals) { return specific_observer{vals...}; }

    template<typename ...Args>
    using deduce_specific_observer_type_t = decltype(deduce_specific_observer_type(std::declval<Args>()...));

} // namespace details
} // namespace rpp
