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
 * @defgroup observers Observers
 * @brief Observer subscribes on Observable and obtains values provided by Observable.
 * @details Observer is kind of wrapper over 3 core functions:
 * - on_next(T) - callback with new emission provided by observable
 * - on_error(err) - failure termination callback with reason of failure of observable (why observable can't continue processing)
 * - on_completed() - succeed termination callback - observable is done, no any future emissions from this
 * @see https://reactivex.io/documentation/observable.html
 * @ingroup rpp
 */

#include <rpp/observers/fwd.hpp>
#include <rpp/observers/base_observer.hpp>
#include <rpp/observers/lambda_observer.hpp>
#include <rpp/observers/dynamic_observer.hpp>
