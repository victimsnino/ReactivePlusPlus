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


#include <rpp/schedulers/fwd.hpp>
#include <rpp/schedulers/details/worker.hpp>
#include <rpp/schedulers/details/utils.hpp>
#include <rpp/utils/functors.hpp>

namespace rpp::schedulers 
{
/**
 * @brief immediately calls provided schedulable or waits for time_point (in the caller-thread)
 * @ingroup schedulers
 */
class immediate final
{
public:
    class worker_strategy
    {
    public:
        template<typename...Args>
        static composite_disposable defer_at(time_point time_point, constraint::schedulable_fn<Args...> auto&& fn, Args&&...args)
        {
            details::immediate_scheduling_while_condition(time_point, rpp::utils::return_true{}, std::forward<decltype(fn)>(fn), std::forward<Args>(args)...);
            return rpp::composite_disposable::empty();
        }

        static time_point now() { return clock_type::now();  }
    };

    static rpp::schedulers::worker<worker_strategy> create_worker()
    {
        return rpp::schedulers::worker<worker_strategy>{};
    }
};
}