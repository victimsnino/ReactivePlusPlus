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

/**
 * @defgroup operators Operators
 * @brief Operators is way to modify observables and extend with some extra custom logic
 * @details By default observable emits some values with some underlying logic. For example, it iterates over vector and
 * emits values. But operators provide ability to make such an stream a bit more complex, for example, emit only SOME
 * values, transform to string and etc. As a result, it would be ANOTHER stream of ANOTHER values, but more suitable for
 * some specific case.
 * @see https://reactivex.io/documentation/operators.html
 * @ingroup rpp
 */

#include <rpp/operators/fwd.hpp>

/**
 * @defgroup transforming_operators Transforming Operators
 * @brief Transforming operators are operators that transform items provided by observable
 * @see https://reactivex.io/documentation/operators.html#transforming
 * @ingroup operators
 */

#include <rpp/operators/buffer.hpp>
#include <rpp/operators/flat_map.hpp>
#include <rpp/operators/group_by.hpp>
#include <rpp/operators/map.hpp>
#include <rpp/operators/scan.hpp>
#include <rpp/operators/subscribe.hpp>
#include <rpp/operators/window.hpp>

/**
 * @defgroup filtering_operators Filtering Operators
 * @brief Filtering operators are operators that emit only part of items that satisfies some condition
 * @see https://reactivex.io/documentation/operators.html#filtering
 * @ingroup operators
 */

#include <rpp/operators/debounce.hpp>
#include <rpp/operators/distinct_until_changed.hpp>
#include <rpp/operators/filter.hpp>
#include <rpp/operators/first.hpp>
#include <rpp/operators/last.hpp>
#include <rpp/operators/skip.hpp>
#include <rpp/operators/take.hpp>
#include <rpp/operators/throttle.hpp>

/**
 * @defgroup conditional_operators Conditional Operators
 * @brief Conditional operators are operators that emit items based on some condition including condition of items from
 * other observables
 * @see https://reactivex.io/documentation/operators.html#conditional
 * @ingroup operators
 */

#include <rpp/operators/take_while.hpp>

/**
 * @defgroup combining_operators Combining Operators
 * @brief Combining operators are operators that combines emissions of multiple observables into same observable by some rule
 * @see https://reactivex.io/documentation/operators.html#combining
 * @ingroup operators
 */

#include <rpp/operators/combine_latest.hpp>
#include <rpp/operators/merge.hpp>
#include <rpp/operators/with_latest_from.hpp>
#include <rpp/operators/start_with.hpp>

/**
 * @defgroup utility_operators Utility Operators
 * @brief Utility operators are operators that provide some extra functionality without changing of original values, but
 * changing of behaviour
 * @see https://reactivex.io/documentation/operators.html#utility
 * @ingroup operators
 */

#include <rpp/operators/as_blocking.hpp>
#include <rpp/operators/delay.hpp>
#include <rpp/operators/observe_on.hpp>
#include <rpp/operators/repeat.hpp>
#include <rpp/operators/subscribe_on.hpp>

/**
 * @defgroup connectable_operators Connectable Operators
 * @brief Connectable operators are operators that provide extra functionality for multicasting of controlling of subscription
 * @see https://reactivex.io/documentation/operators.html#connectable
 * @ingroup operators
 */

#include <rpp/operators/multicast.hpp>
#include <rpp/operators/publish.hpp>
#include <rpp/operators/ref_count.hpp>

/**
 * @defgroup aggregate_operators Aggregate Operators
 * @brief Aggregate operators are operators that operate on the entire sequence of items emitted by an Observable
 * @see https://reactivex.io/documentation/operators.html#mathematical
 * @ingroup operators
 */

#include <rpp/operators/reduce.hpp>
