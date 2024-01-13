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
#include <rpp/operators/details/forwarding_subject.hpp>

#include <cstddef>

namespace rpp
{
template<constraint::decayed_type Type>
using windowed_observable = decltype(std::declval<rpp::operators::details::forwarding_subject<Type>>().get_observable());
}
namespace rpp::operators::details
{
template<rpp::constraint::observer TObserver>
class window_observer_strategy
{
    using Observable = rpp::utils::extract_observer_type_t<TObserver>;
    using value_type = rpp::utils::extract_observable_type_t<Observable>;
    using Subject = forwarding_subject<value_type>;

    static_assert(std::same_as<Observable, decltype(std::declval<Subject>().get_observable())>);

public:
    using preferred_disposable_strategy = rpp::details::observers::none_disposable_strategy;

    window_observer_strategy(TObserver&& observer, size_t count)
        : m_observer{std::move(observer)}
        , m_window_size{std::max(size_t{1}, count)}
    {
        m_observer.set_upstream(m_disposble->add_ref());
    }

    template<typename T>
    void on_next(T&& v) const
    {
        // need to send new subject due to NEW item appeared (we avoid sending new subjects if no any new items)
        if (m_items_in_current_window == m_window_size)
        {
            Subject subject{m_disposble};
            m_subject_data.emplace(subject_data{subject.get_observer(), subject.get_disposable()});
            m_disposble->add(m_subject_data.disposable);
            m_observer.on_next(subject.get_observable());
            m_items_in_current_window = 0;
        }

        m_subject_data->observer.on_next(std::forward<T>(v));

        // cleanup current subject, but don't send due to wait for new value
        if (++m_items_in_current_window == m_window_size)
        {
            m_subject_data->observer.on_completed();
            m_disposble->remove(m_subject_data->disposable);
            m_subject_data.reset();
        }
    }

    void on_error(const std::exception_ptr& err) const
    {
        if (m_subject_data)
            m_subject_data->observer.on_error(err);
        m_observer.on_error(err);
    }

    void on_completed() const
    {
        if (m_subject_data)
            m_subject_data->observer.on_completed();
        m_observer.on_completed();
    }

    void set_upstream(const disposable_wrapper& d) const { m_disposble->add(d); }

    bool is_disposed() const { return m_disposble->is_disposed(); }

private:
    std::shared_ptr<refcount_disposable> m_disposble = std::make_shared<refcount_disposable>();
    RPP_NO_UNIQUE_ADDRESS TObserver      m_observer;

    struct subject_data
    {
        decltype(std::declval<Subject>().get_observer()) observer;
        rpp::disposable_wrapper                          disposable;
    };

    mutable std::optional<subject_data> m_subject_data;
    const size_t                        m_window_size;
    mutable size_t                      m_items_in_current_window = m_window_size;
};

struct window_t : public operators::details::operator_observable_strategy_different_types<window_observer_strategy, rpp::utils::types<>, size_t>
{
    template<rpp::constraint::decayed_type T>
    using result_value = windowed_observable<T>;

    template<rpp::details::observables::constraint::disposable_strategy Prev>
    using updated_disposable_strategy = rpp::details::observables::fixed_disposable_strategy_selector<1>;
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