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

#include <cstddef>

namespace rpp::operators::details
{
template<rpp::constraint::observer TObserver>
class take_last_observer_strategy
{
public:
    using preferred_disposable_strategy = rpp::details::observers::none_disposable_strategy;

    take_last_observer_strategy(TObserver&& observer, size_t count)
        : m_observer{std::move(observer)}
    {
        m_data.reserve(count);
    }

    template<typename T>
    void on_next(T&& v) const
    {
        // handle case "count==0"
        if (m_data.empty())
            return;

        // handle case "count==0"
        if (m_data.size() < m_data.capacity())
        {
            m_data.push_back(std::forward<T>(v));
        }
        else
        {
            m_data[m_current_end] = std::forward<T>(v);
            m_current_end = get_next(m_current_end);
        }
    }

    void on_error(const std::exception_ptr& err) const { m_observer.on_error(err); }

    void on_completed() const 
    { 
        for (size_t i =0; i < m_data.size(); ++i)
        {
            m_observer.on_next(m_data[m_current_end]);
            m_current_end = get_next(m_current_end);
        }

        m_observer.on_completed(); 
    }

    void set_upstream(const disposable_wrapper& d) { m_observer.set_upstream(d); }

    bool is_disposed() const { return m_observer.is_disposed(); }

private:
    size_t get_next(size_t pos) const
    {
        return ++pos >= m_data.size() ? 0 : pos;
    }


private:
    RPP_NO_UNIQUE_ADDRESS TObserver                                     m_observer;
    mutable std::vector<rpp::utils::extract_observer_type_t<TObserver>> m_data{};
    mutable size_t                                                      m_current_end{};
};

struct take_last_t : public operator_observable_strategy_different_types<take_last_observer_strategy, rpp::utils::types<>, size_t>
{
    template<rpp::constraint::decayed_type T>
    using result_value = T;

    template<rpp::details::observables::constraint::disposable_strategy Prev>
    using updated_disposable_strategy = Prev;
};
}

namespace rpp::operators
{
inline auto take_last(size_t count)
{
    return details::take_last_t{count};
}
} // namespace rpp::operators