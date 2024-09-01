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

#include <rpp/schedulers/current_thread.hpp>
#include <rpp/schedulers/details/worker.hpp>

#include <asio/basic_waitable_timer.hpp>
#include <asio/bind_executor.hpp>
#include <asio/strand.hpp>

namespace rppasio::schedulers
{
    /**
     * @brief Asio based scheduler where each worker is assigned an asio `strand` to execute schedulables with the
     * guarantee that none of those `schedulables` will execute concurrently.
     * @details This scheduler can efficiently enable multi-threading execution when running provided io_context
     * in multiple threads. Compared to the `thread_pool` scheduler, a worker is not pinned to a single thread and
     * `schedulables` are instead dynamically dispatched to potentially different io_context threads depending
     * on load.
     * @ingroup asio_schedulers
     */
    class strand
    {
    private:
        class state : public std::enable_shared_from_this<state>
        {
        public:
            state(const asio::io_context::executor_type& executor)
                : m_strand{executor.context()}
            {
            }

            template<rpp::schedulers::constraint::schedulable_handler Handler, typename... Args, rpp::schedulers::constraint::schedulable_fn<Handler, Args...> Fn>
            void defer_for(rpp::schedulers::duration duration, Fn&& fn, Handler&& handler, Args&&... args) const
            {
                if (handler.is_disposed())
                    return;

                if (duration == rpp::schedulers::duration::zero())
                {
                    asio::post(asio::bind_executor(m_strand, [self = this->shared_from_this(), fn = std::forward<Fn>(fn), handler = std::forward<Handler>(handler), ... args = std::forward<Args>(args)]() mutable {
                        if (handler.is_disposed())
                            return;

                        if (const auto new_duration = fn(handler, args...))
                            self->defer_for(new_duration->value, std::move(fn), std::move(handler), std::move(args)...);
                    }));
                }
                else
                {
                    auto timer = std::make_shared<asio::basic_waitable_timer<rpp::schedulers::clock_type>>(m_strand.context(), duration);
                    timer->async_wait(asio::bind_executor(m_strand, [self = this->shared_from_this(), timer, fn = std::forward<Fn>(fn), handler = std::forward<Handler>(handler), ... args = std::forward<Args>(args)](const asio::error_code& ec) mutable {
                        if (ec || handler.is_disposed())
                            return;

                        if (const auto new_duration = fn(handler, args...))
                            self->defer_for(new_duration->value, std::move(fn), std::move(handler), std::move(args)...);
                    }));
                }
            }

        private:
            asio::io_context::strand m_strand;
        };

        class worker_strategy
        {
        public:
            explicit worker_strategy(const asio::io_context::executor_type& executor)
                : m_state{std::make_shared<state>(executor)}
            {
            }

            template<rpp::schedulers::constraint::schedulable_handler Handler, typename... Args, rpp::schedulers::constraint::schedulable_fn<Handler, Args...> Fn>
            void defer_for(rpp::schedulers::duration duration, Fn&& fn, Handler&& handler, Args&&... args) const
            {
                m_state->defer_for(duration, std::forward<Fn>(fn), std::forward<Handler>(handler), std::forward<Args>(args)...);
            }

            static constexpr rpp::schedulers::details::none_disposable get_disposable() { return {}; }
            static rpp::schedulers::time_point                         now() { return rpp::schedulers::clock_type::now(); }

        private:
            std::shared_ptr<state> m_state;
        };

    public:
        explicit strand(asio::io_context::executor_type executor)
            : m_executor{std::move(executor)}
        {
        }

        auto create_worker() const
        {
            return rpp::schedulers::worker<worker_strategy>{m_executor};
        }

        asio::io_context::executor_type m_executor;
    };
} // namespace rppasio::schedulers
