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
#include <rpp/subscriptions/subscription_base.hpp>  // lifetime
#include <rppqt/utils/exceptions.hpp>

#include <chrono>
#include <concepts>

#include <QCoreApplication>
#include <QTimer>

namespace rppqt::schedulers
{
/**
 * \brief Schedule provided schedulables to main GUI QT thread (where QApplication placed)
 * \ingroup qt_schedulers
 */
class main_thread_scheduler final : public rpp::schedulers::details::scheduler_tag
{
private:
    class worker_strategy;
    using main_thread_schedulable = rpp::schedulers::schedulable_wrapper<worker_strategy>;

    class worker_strategy
    {
    public:
        worker_strategy(const rpp::subscription_base& sub)
            : m_sub{sub} {}

        bool is_subscribed() const { return m_sub.is_subscribed(); }

        void defer_at(rpp::schedulers::time_point time_point, rpp::schedulers::constraint::schedulable_fn auto&& fn) const
        {
            defer_at(time_point, main_thread_schedulable{*this, time_point, std::forward<decltype(fn)>(fn)});
        }

        void defer_at(rpp::schedulers::time_point time_point, main_thread_schedulable&& fn) const
        {
            if (!m_sub.is_subscribed())
                return;

            const auto application = QCoreApplication::instance();
            if (!application)
                throw utils::no_active_qapplication{
                    "Pointer to application is null. Create QApplication before using main_thread_scheduler!"};

            const auto duration = std::max(std::chrono::milliseconds{0}, std::chrono::duration_cast<std::chrono::milliseconds>(time_point - now()));
            QTimer::singleShot(duration, application, std::move(fn));
        }

        static rpp::schedulers::time_point now() { return rpp::schedulers::clock_type::now(); }
    private:
        rpp::subscription_base m_sub;
    };

public:
    static auto create_worker(const rpp::subscription_base& sub = {})
    {
        return rpp::schedulers::worker<worker_strategy>{sub};
    }
};
} // namespace rppqt::schedulers
