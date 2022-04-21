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

#include <rpp/observers/constraints.hpp>
#include <rpp/observers/interface_observer.hpp>
#include <rpp/observers/specific_observer.hpp>
#include <rpp/observers/type_traits.hpp>
#include <rpp/utils/function_traits.hpp>
#include <rpp/utils/functors.hpp>

#include <memory>

namespace rpp
{
/**
 * \brief Dynamic (type-erased) version of observer (comparing to specific_observer)
 * \details It uses type-erasure mechanism to hide types of OnNext, OnError and OnCompleted callbacks. But it has higher cost in the terms of performance due to usage of heap.
 * Use it only when you need to store observer as member variable or something like this. In other cases prefer using "auto" to avoid converting to dynamic_observer
 * \tparam T is type of value handled by this observer
 * \ingroup observers
 */
template<constraint::decayed_type T>
class dynamic_observer final : public interface_observer<T>
{
public:
    template<constraint::on_next_fn<T>   OnNext      = utils::empty_function_t<T>,
             constraint::on_error_fn     OnError     = utils::empty_function_t<std::exception_ptr>,
             constraint::on_completed_fn OnCompleted = utils::empty_function_t<>>
    dynamic_observer(OnNext&& on_next = {}, OnError&& on_error = {}, OnCompleted&& on_completed = {})
        : m_state{make_shared_state(std::forward<OnNext>(on_next),
                                    std::forward<OnError>(on_error),
                                    std::forward<OnCompleted>(on_completed))} { }

    dynamic_observer(constraint::on_next_fn<T> auto&& on_next, constraint::on_completed_fn auto&& on_completed)
        : dynamic_observer{std::forward<decltype(on_next)>(on_next),
                           utils::empty_function_t<std::exception_ptr>{},
                           std::forward<decltype(on_completed)>(on_completed)} {}

    template<constraint::observer TObserver> 
        requires (!std::is_same_v<std::decay_t<TObserver>, dynamic_observer<T>>)
    dynamic_observer(TObserver&& obs)
        : m_state{std::make_shared<std::decay_t<TObserver>>(std::forward<TObserver>(obs))} {}

    dynamic_observer(const dynamic_observer<T>&)     = default;
    dynamic_observer(dynamic_observer<T>&&) noexcept = default;

    void on_next(const T& v) const override                     { m_state->on_next(v);              }
    void on_next(T&& v) const override                          { m_state->on_next(std::move(v));   }
    void on_error(const std::exception_ptr& err) const override { m_state->on_error(err);           }
    void on_completed() const override                          { m_state->on_completed();          }

    /**
     * \brief Do nothing for rpp::dynamic_observer. Created only for unification of interfaces with rpp::specific_observer
     */
    const dynamic_observer<T>& as_dynamic() const { return *this; }

private:
    template<typename ...Args>
    static auto make_shared_state(Args&&...args)
    {
        return std::make_shared<specific_observer<T, std::decay_t<Args>...>>(std::forward<Args>(args)...);
    }
    
    std::shared_ptr<interface_observer<T>> m_state;
};

template<constraint::observer TObserver>
dynamic_observer(TObserver) -> dynamic_observer<utils::extract_observer_type_t<TObserver>>;

template<typename OnNext, typename ...Args>
dynamic_observer(OnNext, Args...) -> dynamic_observer<utils::decayed_function_argument_t<OnNext>>;
} // namespace rpp
