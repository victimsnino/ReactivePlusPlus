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
 * @defgroup observables Observables
 * @brief Observable is the source of any Reactive Stream.
 * @details Observable provides ability to subscribe observer on stream of events. After subscription observable would emit values to observer.
 * @see https://reactivex.io/documentation/observable.html
 * @ingroup rpp
 */

#include <rpp/observables/fwd.hpp>

#include <rpp/observables/observable.hpp>
#include <rpp/observables/blocking_observable.hpp>
#include <rpp/observables/connectable_observable.hpp>
#include <rpp/observables/dynamic_observable.hpp>
#include <rpp/observables/grouped_observable.hpp>
#include <rpp/observables/variant_observable.hpp>
