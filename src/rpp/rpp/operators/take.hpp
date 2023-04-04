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

#include <rpp/operators/fwd.hpp>
#include <rpp/operators/details/strategy.hpp>
#include <cstddef>

namespace rpp::operators::details
{
struct take_observer_strategy
{
    size_t count{};

};
}

namespace rpp::operators
{
/**
* @brief Emit only first `count` items provided by observable, then send `on_completed`
*
* @marble take
{
    source observable  : +--1-2-3-4-5-6-|
    operator "take(3)" : +--1-2-3|
}
* @details Actually this operator just emits emissions while counter is not zero and decrements counter on each emission
*
* @param count amount of items to be emitted. 0 - instant complete
* @return new specific_observable with the Take operator as most recent operator.
* @warning #include <rpp/operators/take.hpp>
*
* @par Example:
* @snippet take.cpp take
*
* @ingroup filtering_operators
* @see https://reactivex.io/documentation/operators/take.html
*/
class take
{
public:
    take(size_t count)
        : m_count{count} {}

    template<constraint::observable TObservable>
    auto operator()(TObservable&& observable) const
    {
        return details::operator_observable<TObservable, details::take_observer_strategy, size_t>{std::forward<TObservable>(observable), m_count};
    }

private:
    size_t m_count;
};
} // namespace rpp::operators