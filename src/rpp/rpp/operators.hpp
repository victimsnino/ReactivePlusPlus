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

 #include <rpp/operators/map.hpp>
 #include <rpp/operators/subscribe.hpp>


/**
 * @defgroup filtering_operators Filtering Operators
 * @brief Filtering operators are operators that emit only part of items that satisfies some condition
 * @see https://reactivex.io/documentation/operators.html#filtering
 * @ingroup operators
 */
 #include <rpp/operators/take.hpp>