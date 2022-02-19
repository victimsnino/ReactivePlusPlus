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
#include <rpp/observers/interface_observer.h>
#include <rpp/utils/function_traits.h>
#include <rpp/utils/functors.h>
#include <rpp/utils/type_traits.h>

#include <memory>

namespace rpp
{
template<typename T>
class dynamic_observer final : public interface_observer<T>
{
    static_assert(std::is_same_v<std::decay_t<T>, T>, "Type should be decayed to match with decayed observable types");

public:
    template<typename OnNext = utils::empty_function_t<T>,
             typename OnError = utils::empty_function_t<std::exception_ptr>,
             typename OnCompleted = utils::empty_function_t<>,
             typename = std::enable_if_t<utils::is_observer_constructible_v<T, OnNext, OnError, OnCompleted>>>
    dynamic_observer(OnNext&& on_next = {}, OnError&& on_error = {}, OnCompleted&& on_completed = {})
        : m_state{make_shared_state(std::forward<OnNext>(on_next),
                                    std::forward<OnError>(on_error),
                                    std::forward<OnCompleted>(on_completed))} { }

    template<typename OnNext,
             typename OnCompleted,
             typename = std::enable_if_t<utils::is_observer_constructible_v<T, OnNext, utils::empty_function_t<std::exception_ptr>, OnCompleted>>>
    dynamic_observer(OnNext&& on_next, OnCompleted&& on_completed)
        : dynamic_observer{std::forward<OnNext>(on_next),
                           std::forward<OnCompleted>(on_completed)} {}

    template<typename OnNext, typename OnError, typename OnCompleted>
    dynamic_observer(const specific_observer<T, OnNext, OnError, OnCompleted>& obs)
        : m_state{std::make_shared<specific_observer<T, OnNext, OnError, OnCompleted>>(obs)} {}

    template<typename OnNext, typename OnError, typename OnCompleted>
    dynamic_observer(specific_observer<T, OnNext, OnError, OnCompleted>&& obs)
        : m_state{std::make_shared<specific_observer<T, OnNext, OnError, OnCompleted>>(std::move(obs))} {}

    dynamic_observer(const dynamic_observer<T>&)     = default;
    dynamic_observer(dynamic_observer<T>&&) noexcept = default;

    void on_next(const T& v) const override                     { m_state->on_next(v);              }
    void on_next(T&& v) const override                          { m_state->on_next(std::move(v));   }
    void on_error(const std::exception_ptr& err) const override { m_state->on_error(err);           }
    void on_completed() const override                          { m_state->on_completed();          }

private:
    template<typename ...Args>
    static auto make_shared_state(Args&&...args)
    {
        return std::make_shared<specific_observer<T, std::decay_t<Args>...>>(std::forward<Args>(args)...);
    }
    
    std::shared_ptr<interface_observer<T>> m_state;
};

template<typename OnNext, typename ...Args>
dynamic_observer(OnNext, Args...) -> dynamic_observer<std::decay_t<utils::function_argument_t<OnNext>>>;
} // namespace rpp
