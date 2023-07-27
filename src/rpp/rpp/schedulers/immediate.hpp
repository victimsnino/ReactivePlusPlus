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
 * @par Example
 * ```cpp
   auto worker = rpp::schedulers::immediate::create_worker();
   worker.schedule([&worker](const auto& observer)
   {
       std::cout << "Task 1 starts" << std::endl;
   
       worker.schedule([&worker](const auto& observer)
       {
           std::cout << "Task 2 starts" << std::endl;
           worker.schedule([](const auto&)
           {
               std::cout << "Task 4" << std::endl;
               return rpp::schedulers::optional_duration{};
           }, observer);
           std::cout << "Task 2 ends" << std::endl;
           return rpp::schedulers::optional_duration{};
       }, observer);
   
       worker.schedule([](const auto&)
       {
           std::cout << "Task 3" << std::endl;
           return rpp::schedulers::optional_duration{};
       }, observer);
   
       std::cout << "Task 1 ends" << std::endl;
       return rpp::schedulers::optional_duration{};
   }, obse
 *
 * Would lead to:
 * - "Task 1 starts"
 * - "Task 2 starts"
 * - "Task 4"
 * - "Task 2 ends"
 * - "Task 3"
 * - "Task 1 ends"
 *
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

        static rpp::disposable_wrapper get_disposable() {return rpp::disposable_wrapper{}; }
    };

    static rpp::schedulers::worker<worker_strategy> create_worker()
    {
        return rpp::schedulers::worker<worker_strategy>{};
    }
};
}
