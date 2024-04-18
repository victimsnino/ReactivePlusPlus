//                   ReactivePlusPlus library
//
//           Copyright Aleksey Loginov 2022 - present.
//  Distributed under the Boost Software License, Version 1.0.
//     (See accompanying file LICENSE_1_0.txt or copy at
//           https://www.boost.org/LICENSE_1_0.txt)
//
//  Project home: https://github.com/victimsnino/ReactivePlusPlus

#pragma once

#include <rpp/schedulers/fwd.hpp>

#include <rpp/defs.hpp>
#include <rpp/schedulers/details/utils.hpp>
#include <rpp/utils/constraints.hpp>
#include <rpp/utils/tuple.hpp>
#include <rpp/utils/utils.hpp>

#include <condition_variable>
#include <exception>
#include <memory>
#include <mutex>
#include <optional>
#include <utility>

namespace rpp::schedulers::details
{
    class schedulable_base
    {
    public:
        explicit schedulable_base(const time_point& time_point)
            : m_time_point{time_point}
        {
        }

        virtual ~schedulable_base() noexcept = default;

        virtual std::optional<time_point> operator()() noexcept        = 0;
        virtual bool                      is_disposed() const noexcept = 0;

        time_point get_timepoint() const { return m_time_point; }

        void set_timepoint(const time_point& timepoint) { m_time_point = timepoint; }

        const std::shared_ptr<schedulable_base>& get_next() const { return m_next; }

        void set_next(std::shared_ptr<schedulable_base>&& next) { m_next = std::move(next); }

        void update_next(std::shared_ptr<schedulable_base>&& next)
        {
            if (next)
                next->set_next(std::move(m_next));
            m_next = std::move(next);
        }

    private:
        std::shared_ptr<schedulable_base> m_next{};
        time_point                        m_time_point;
    };

    template<typename NowStrategy, rpp::constraint::decayed_type Fn, rpp::schedulers::constraint::schedulable_handler Handler, rpp::constraint::decayed_type... Args>
        requires constraint::schedulable_fn<Fn, Handler, Args...>
    class specific_schedulable final : public schedulable_base
    {
    public:
        template<rpp::constraint::decayed_same_as<Fn> TFn, typename... TArgs>
        explicit specific_schedulable(const time_point& time_point, TFn&& in_fn, TArgs&&... in_args)
            : schedulable_base{time_point}
            , m_args{std::forward<TArgs>(in_args)...}
            , m_fn{std::forward<TFn>(in_fn)}
        {
        }

        std::optional<time_point> operator()() noexcept override
        {
            try
            {
                if (const auto res = m_args.apply(m_fn))
                {
                    if constexpr (constraint::schedulable_delay_from_now_fn<Fn, Handler, Args...>)
                    {
                        return NowStrategy::now() + res->value;
                    }
                    else if constexpr (constraint::schedulable_delay_to_fn<Fn, Handler, Args...>)
                    {
                        return res->value;
                    }
                    else
                    {
                        static_assert(constraint::schedulable_delay_from_this_timepoint_fn<Fn, Handler, Args...>);
                        return get_timepoint() + res->value;
                    }
                }
            }
            catch (...)
            {
                m_args.template get<0>().on_error(std::current_exception());
            }
            return std::nullopt;
        }

        bool is_disposed() const noexcept override { return m_args.template get<0>().is_disposed(); }

    private:
        RPP_NO_UNIQUE_ADDRESS rpp::utils::tuple<Handler, Args...> m_args;
        RPP_NO_UNIQUE_ADDRESS Fn                                  m_fn;
    };

    template<typename Mutex>
    class optional_mutex
    {
    public:
        optional_mutex() {}

        optional_mutex(Mutex* mutex)
            : m_mutex{mutex}
        {
        }

        void lock() const
        {
            if (m_mutex)
                m_mutex->lock();
        }

        void unlock() const
        {
            if (m_mutex)
                m_mutex->unlock();
        }

    private:
        Mutex* m_mutex{};
    };

    struct shared_queue_data
    {
        std::condition_variable_any cv{};
        std::recursive_mutex        mutex{};
    };

    template<typename NowStrategy>
    class schedulables_queue
    {
    public:
        schedulables_queue()                              = default;
        schedulables_queue(const schedulables_queue&)     = delete;
        schedulables_queue(schedulables_queue&&) noexcept = default;

        schedulables_queue& operator=(const schedulables_queue& other)     = delete;
        schedulables_queue& operator=(schedulables_queue&& other) noexcept = default;

        schedulables_queue(std::weak_ptr<shared_queue_data> shared_data)
            : m_shared_data{std::move(shared_data)}
        {
        }

        template<rpp::schedulers::constraint::schedulable_handler Handler, typename... Args, constraint::schedulable_fn<Handler, Args...> Fn>
        void emplace(const time_point& timepoint, Fn&& fn, Handler&& handler, Args&&... args)
        {
            using schedulable_type = specific_schedulable<NowStrategy, std::decay_t<Fn>, std::decay_t<Handler>, std::decay_t<Args>...>;

            emplace_impl(std::make_shared<schedulable_type>(timepoint, std::forward<Fn>(fn), std::forward<Handler>(handler), std::forward<Args>(args)...));
        }

        void emplace(const time_point& timepoint, std::shared_ptr<schedulable_base>&& schedulable)
        {
            if (!schedulable)
                return;

            schedulable->set_timepoint(timepoint);
            emplace_impl(std::move(schedulable));
        }

        bool is_empty() const { return !m_head; }

        std::shared_ptr<schedulable_base> pop()
        {
            return std::exchange(m_head, m_head->get_next());
        }

        const std::shared_ptr<schedulable_base>& top() const
        {
            return m_head;
        }

    private:
        void emplace_impl(std::shared_ptr<schedulable_base>&& schedulable)
        {
            // needed in case of new_thread and current_thread shares same queue
            const auto                       s = m_shared_data.lock();
            const rpp::utils::finally_action _{[&] {
                if (s)
                    s->cv.notify_one();
            }};

            optional_mutex<std::recursive_mutex> mutex{s ? &s->mutex : nullptr};
            std::lock_guard                      lock{mutex};

            if (!m_head || schedulable->get_timepoint() < m_head->get_timepoint())
            {
                schedulable->set_next(std::move(m_head));
                m_head = std::move(schedulable);
                return;
            }

            schedulable_base* current = m_head.get();
            while (const auto& next = current->get_next())
            {
                if (schedulable->get_timepoint() < next->get_timepoint())
                    break;
                current = next.get();
            }

            current->update_next(std::move(schedulable));
        }

    private:
        std::shared_ptr<schedulable_base> m_head{};
        std::weak_ptr<shared_queue_data>  m_shared_data{};
    };
} // namespace rpp::schedulers::details
