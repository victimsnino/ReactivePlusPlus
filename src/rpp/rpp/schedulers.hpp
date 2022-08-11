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
 * \defgroup schedulers Schedulers
 * \brief Scheduler is the way to introduce multi-threading in your application via RPP
 * \see https://reactivex.io/documentation/scheduler.html
 */

#include <rpp/schedulers/immediate_scheduler.hpp>
#include <rpp/schedulers/new_thread_scheduler.hpp>
#include <rpp/schedulers/run_loop_scheduler.hpp>
#include <rpp/schedulers/trampoline_scheduler.hpp>