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
class buffer_observer_strategy
{
    using Container = rpp::utils::extract_observer_type_t<TObserver>;
    using ValueType = typename Container::value_type;
    static_assert(std::same_as<Container, std::vector<ValueType>>);
    
public:
    using DisposableStrategyToUseWithThis = rpp::details::none_disposable_strategy;

    buffer_observer_strategy(TObserver&& observer, size_t count)
        : m_observer{std::move(observer)}
    {
        m_bucket.reserve(std::max(size_t{1}, count));
    }

    template<typename T>
    void on_next(T&& v) const
    {
        m_bucket.push_back(std::forward<T>(v));
        if (m_bucket.size() == m_bucket.capacity())
        {
            m_observer.on_next(std::move(m_bucket));
            m_bucket.clear();
        }
    }

    void on_error(const std::exception_ptr& err) const { m_observer.on_error(err); }

    void on_completed() const 
    { 
        if (!m_bucket.empty())
            m_observer.on_next(std::move(m_bucket));
        m_observer.on_completed(); 
    }

    void set_upstream(const disposable_wrapper& d) { m_observer.set_upstream(d); }

    bool is_disposed() const { return m_observer.is_disposed(); }

private:
    RPP_NO_UNIQUE_ADDRESS TObserver m_observer;
    mutable std::vector<ValueType>  m_bucket;
};

struct buffer_t : public operators::details::operator_observable_strategy_diffferent_types<buffer_observer_strategy, rpp::utils::types<>, size_t>
{
    template<rpp::constraint::decayed_type T>
    using ResultValue = std::vector<T>;
};
}

namespace rpp::operators
{
/**
 * @brief Periodically gather emissions emitted by an original Observable into bundles and emit these bundles rather than emitting
 * the items one at a time
 *
 * @marble buffer
     {
         source observable    : +-1-2-3-|
         operator "buffer(2)" : +---{1,2}-{3}-|
     }
 *
 * @details The resulting bundle is `std::vector<Type>` of requested size. Actually it is similar to `window()` operator, but it emits vectors instead of observables.
 *
 * @param count number of items being bundled.
 * @warning #include <rpp/operators/buffer.hpp>
 * 
 * @par Example:
 * @snippet buffer.cpp buffer
 *
 * @ingroup transforming_operators
 * @see https://reactivex.io/documentation/operators/buffer.html
 */
inline auto buffer(size_t count)
{
    return details::buffer_t{count};
}
} // namespace rpp::operators