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

#include <rppqt/schedulers/fwd.hpp>                 // own forwarding
#include <rpp/schedulers/details/worker.hpp>        // worker
#include <rppqt/utils/exceptions.hpp>

#include <chrono>
#include <concepts>

#include <QCoreApplication>
#include <QTimer>

namespace rppqt::schedulers
{
/**
 * @brief Schedule provided schedulables to main GUI QT thread (where QApplication placed)
 * @ingroup qt_schedulers
 */
class main_thread_scheduler final
{
private:
    class worker_strategy
    {
    public:
        template<rpp::schedulers::constraint::schedulable_handler Handler, typename... Args, rpp::schedulers::constraint::schedulable_fn<Handler, Args...> Fn>
        static void defer_for(rpp::schedulers::duration duration, Fn&& fn, Handler&& handler, Args&&... args)
        {
            const auto application = QCoreApplication::instance();
            if (!application)
                throw utils::no_active_qapplication{"Pointer to application is null. Create QApplication before using main_thread_scheduler!"};

            QTimer::singleShot(std::chrono::duration_cast<std::chrono::milliseconds>(duration), application, [fn = std::forward<Fn>(fn), handler = std::forward<Handler>(handler), ... args = std::forward<Args>(args)]() mutable {
                if (const auto new_duration = fn(handler, args...))
                    defer_for(new_duration.value(), std::move(fn), std::move(handler), std::move(args)...);
            });
        }

        static constexpr rpp::schedulers::details::none_disposable get_disposable() { return {}; }

        static rpp::schedulers::time_point now() { return rpp::schedulers::clock_type::now(); }
    };

public:
    static auto create_worker()
    {
        return rpp::schedulers::worker<worker_strategy>{};
    }
};
} // namespace rppqt::schedulers
