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

/**
* \defgroup operators Operators
* \brief Operators is way to modify observables and extend with some extra custom logic
* \see https://reactivex.io/documentation/operators.html 
* \ingroup rpp
*/

/**
 * \defgroup transforming_operators Transforming Operators
 * \brief Transforming operators are operators that transform items provided by observable
 * \see https://reactivex.io/documentation/operators.html#transforming
 * \ingroup operators
 */

#include <rpp/operators/buffer.hpp>
#include <rpp/operators/flat_map.hpp>
#include <rpp/operators/group_by.hpp>
#include <rpp/operators/map.hpp>
#include <rpp/operators/scan.hpp>
#include <rpp/operators/window.hpp>

/**
 * \defgroup filtering_operators Filtering Operators
 * \brief Filtering operators are operators that emit only part of items that satisfies some condition
 * \see https://reactivex.io/documentation/operators.html#filtering
 * \ingroup operators
 */

#include <rpp/operators/debounce.hpp>
#include <rpp/operators/distinct_until_changed.hpp>
#include <rpp/operators/filter.hpp>
#include <rpp/operators/first.hpp>
#include <rpp/operators/last.hpp>
#include <rpp/operators/sample.hpp>
#include <rpp/operators/skip.hpp>
#include <rpp/operators/take.hpp>
#include <rpp/operators/take_last.hpp>

/**
 * \defgroup conditional_operators Conditional Operators
 * \brief Conditional operators are operators that emit items based on some condition including condition of items from other observables
 * \see https://reactivex.io/documentation/operators.html#conditional
 * \ingroup operators
 */

#include <rpp/operators/take_until.hpp>
#include <rpp/operators/take_while.hpp>

/**
 * \defgroup combining_operators Combining Operators
 * \brief Combining operators are operators that combines emissions of multiple observables into same observable by some rule
 * \see https://reactivex.io/documentation/operators.html#combining
 * \ingroup operators
 */

#include <rpp/operators/combine_latest.hpp>
#include <rpp/operators/merge.hpp>
#include <rpp/operators/start_with.hpp>
#include <rpp/operators/switch_map.hpp>
#include <rpp/operators/switch_on_next.hpp>
#include <rpp/operators/with_latest_from.hpp>

/**
* \defgroup aggregate_operators Aggregate Operators
* \brief Aggregate operators are operators on the entire sequence of items provided by observable
* \see https://reactivex.io/documentation/operators.html#mathematical
* \ingroup operators
*/

#include <rpp/operators/concat.hpp>
#include <rpp/operators/reduce.hpp>

/**
 * \defgroup utility_operators Utility Operators
 * \brief Utility operators are operators that provide some extra functionality without changing of original values, but changing of behaviour
 * \see https://reactivex.io/documentation/operators.html#utility
 * \ingroup operators
 */

#include <rpp/operators/delay.hpp>
#include <rpp/operators/do.hpp>
#include <rpp/operators/observe_on.hpp>
#include <rpp/operators/repeat.hpp>
#include <rpp/operators/subscribe_on.hpp>
#include <rpp/operators/timeout.hpp>

/**
 * \defgroup connectable_operators Connectable Operators
 * \brief Connectable operators are operators that provide extra functionality for multicasting of controlling of subscription
 * \see https://reactivex.io/documentation/operators.html#connectable
 * \ingroup operators
 */

#include <rpp/operators/multicast.hpp>
#include <rpp/operators/publish.hpp>
#include <rpp/operators/ref_count.hpp>

/**
 * \defgroup error_handling_operators Error handling Operators
 * \brief Error handling operators Operators that help to recover from error notifications from an Observable.
 * \see https://reactivex.io/documentation/operators.html#error
 * \ingroup operators
 */

#include <rpp/operators/on_error_resume_next.hpp>
