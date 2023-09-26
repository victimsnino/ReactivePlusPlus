//                  ReactivePlusPlus library
//
//          Copyright Aleksey Loginov 2023 - present.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)
//
// Project home: https://github.com/victimsnino/ReactivePlusPlus
//

#pragma once

#include <rpp/operators/fwd.hpp>

#include <rpp/defs.hpp>
#include <rpp/operators/details/strategy.hpp>
#include <rpp/disposables/refcount_disposable.hpp>

#include <cstddef>

namespace rpp::operators::details
{
template<rpp::constraint::observer TObserver>
struct window_disposable : public refcount_disposable
{
    using Observable = rpp::utils::extract_observer_type_t<TObserver>;
    using ValueType = rpp::utils::extract_observable_type_t<Observable>;
    using Subject = subjects::publish_subject<ValueType>;

    window_disposable(TObserver&& observer, size_t count)
        : observer{std::move(observer)}
        , window_size{std::max(size_t{1}, count)}
    {
    }

    RPP_NO_UNIQUE_ADDRESS TObserver observer;
    Subject                         subject{};
    const size_t                    window_size;
    mutable size_t                  items_in_current_window = window_size;
};

template<rpp::constraint::observer TObserver>
class window_observer_strategy
{
    using Observable = rpp::utils::extract_observer_type_t<TObserver>;
    using ValueType = rpp::utils::extract_observable_type_t<Observable>;
    using Subject = subjects::publish_subject<ValueType>;
    static_assert(std::same_as<Observable, decltype(std::declval<Subject>().get_observable())>);
    
public:
    using DisposableStrategyToUseWithThis = rpp::details::none_disposable_strategy;

    window_observer_strategy(TObserver&& observer, size_t count)
        : m_state{std::make_shared<window_disposable<TObserver>>(std::move(observer), count)}
    {
        m_state->observer.set_upstream(rpp::disposable_wrapper::from_weak(m_state));
    }

    template<typename T>
    void on_next(T&& v) const
    {
        // need to send new subject due to NEW item appeared (we avoid sending new subjects if no any new items)
        if (m_state->items_in_current_window == m_state->window_size)
        {
            if (m_state->subject.get_disposable().is_disposed())
                m_state->subject = Subject{};
            
            m_state->subject.get_observer().set_upstream(rpp::disposable_wrapper::from_weak(m_state));
            m_state->observer.on_next(m_state->subject.get_observable());
            m_state->items_in_current_window = 0;
        }

        ++m_state->items_in_current_window;
        m_state->subject.get_subscriber().on_next(std::forward<T>(v));

        // cleanup current subject, but don't send due to wait for new value
        if (m_state->items_in_current_window == m_state->window_size)
            m_state->subject.get_subscriber().on_completed();
    }

    void on_error(const std::exception_ptr& err) const
    {
        m_state->subject.get_observer().on_error(err);
        m_state->observer.on_error(err);
    }

    void on_completed() const
    {
        m_state->subject.get_observer().on_completed();
        m_state->observer.on_completed();
    }

    void set_upstream(const disposable_wrapper& d) { m_state->add(d); }

    bool is_disposed() const { return m_state->is_disposed_underlying(); }

private:
    std::shared_ptr<window_disposable<TObserver>> m_state;
};

template<constraint::decayed_type Type>
using windowed_observable = decltype(std::declval<subjects::publish_subject<Type>>().get_observable());

struct window_t : public operators::details::operator_observable_strategy_diffferent_types<window_observer_strategy, rpp::utils::types<>, size_t>
{
    template<rpp::constraint::decayed_type T>
    using ResultValue = windowed_observable<T>;
};
}

namespace rpp::operators
{
/**
 * @brief Subdivide original observable into sub-observables (windowed observables) and emit sub-observables of items instead of original items
 * 
 * @marble window
   {
       source observable    :  +-1-2-3-4-5-|
  
       operator "window(2)" : 
                           {   
                               .+1-2|
                               .....+3-4|
                               .........+5-|
                           }
   }
 *
 * @details Actually it is similar to `buffer` but it emits observable instead of container.
 *
 * @param window_size amount of items which every observable would have
 *
 * @warning #include <rpp/operators/window.hpp>
 * 
 * @par Example
 * @snippet window.cpp window
 *   
 * @ingroup transforming_operators
 * @see https://reactivex.io/documentation/operators/window.html
 */
inline auto window(size_t count)
{
    return details::window_t{count};
}
} // namespace rpp::operators