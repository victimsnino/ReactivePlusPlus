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

#include <rpp/observers/constraints.h>
#include <rpp/observers/fwd.h>
#include <rpp/observers/interface_observer.h>
#include <rpp/observers/type_traits.h>
#include <rpp/observers/details/base_observer.h>
#include <rpp/utils/function_traits.h>
#include <rpp/utils/functors.h>

#include <memory>

namespace rpp
{
namespace details
{
    template<constraint::decayed_type T>
    struct dynamic_observer_callables
    {
        template<constraint::on_next_fn<T>   OnNext      = utils::empty_function_t<T>,
                 constraint::on_error_fn     OnError     = utils::empty_function_t<std::exception_ptr>,
                 constraint::on_completed_fn OnCompleted = utils::empty_function_t<>>
        dynamic_observer_callables(OnNext&& on_next = {}, OnError&& on_error = {}, OnCompleted&& on_completed = {})
            : m_state{make_shared_state(std::forward<OnNext>(on_next),
                                        std::forward<OnError>(on_error),
                                        std::forward<OnCompleted>(on_completed))} {}

        dynamic_observer_callables(const std::shared_ptr<interface_observer<T>>& state)
            : m_state{state}{}

        const interface_observer<T>* operator->() const { return m_state.get(); }

    private:
        template<typename ...Args>
        static auto make_shared_state(Args&&...args)
        {
            return std::make_shared<specific_observer<T, std::decay_t<Args>...>>(std::forward<Args>(args)...);
        }

        std::shared_ptr<interface_observer<T>> m_state;
    };
}
/**
 * \brief Dynamic (type-erased) version of observer (comparing to specific_observer)
 * \details It uses type-erasure mechanism to hide types of OnNext, OnError and OnCompleted callbacks. But it has higher cost in the terms of performance due to usage of heap.
 * Use it only when you need to store observer as member variable or something like this. In other cases prefer using "auto" to avoid converting to dynamic_observer
 * \tparam T is type of value handled by this observer
 */
template<constraint::decayed_type T>
class dynamic_observer final : public details::base_observer<T, details::dynamic_observer_callables<T>>
{
public:
    using parent = details::base_observer<T, details::dynamic_observer_callables<T>>;
    using parent::parent;

    template<constraint::observer TObserver>
        requires (!std::is_same_v<std::decay_t<TObserver>, dynamic_observer<T>>)
    dynamic_observer(TObserver&& obs)
        : parent{details::dynamic_observer_callables<T>{std::make_shared<std::decay_t<TObserver>>(std::forward<TObserver>(obs))}} {}

    /**
     * \brief Do nothing for rpp::dynamic_observer. Created only for unification of interfaces with rpp::specific_observer
     */
    const dynamic_observer<T>& as_dynamic() const { return *this; }
};

template<constraint::observer TObserver>
dynamic_observer(TObserver) -> dynamic_observer<utils::extract_observer_type_t<TObserver>>;

template<typename OnNext, typename ...Args>
dynamic_observer(OnNext, Args...) -> dynamic_observer<utils::decayed_function_argument_t<OnNext>>;
} // namespace rpp
