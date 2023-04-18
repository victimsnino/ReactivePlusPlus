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
#include <rpp/disposables/disposable_wrapper.hpp>

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
        template<rpp::constraint::observer TObs, typename...Args, constraint::schedulable_fn<TObs, Args...> Fn>
        static void defer_for(duration duration, Fn&& fn, TObs&& obs, Args&&...args)
        {
            details::immediate_scheduling_while_condition(duration, rpp::utils::return_true{}, std::forward<Fn>(fn), std::forward<TObs>(obs), std::forward<Args>(args)...);
        }

        rpp::disposable_wrapper get_disposable() const {return rpp::disposable_wrapper{}; }
    };

    static rpp::schedulers::worker<worker_strategy> create_worker()
    {
        return rpp::schedulers::worker<worker_strategy>{};
    }
};
}
