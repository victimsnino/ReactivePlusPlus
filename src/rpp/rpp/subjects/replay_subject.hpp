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
#include <rpp/subjects/fwd.hpp>

#include <rpp/observers/observer.hpp>
#include <rpp/subjects/details/base_subject.hpp>
#include <rpp/subjects/details/subject_state.hpp>

#include <list>
#include <optional>
#include <utility>

namespace rpp::subjects::details
{
    template<rpp::constraint::decayed_type Type, bool Serialized>
    class replay_strategy
    {
        struct replay_state final : public subject_state<Type>
        {
            replay_state(std::optional<size_t> count, std::optional<rpp::schedulers::duration> duration)
                : count(count)
                , duration(duration)
            {
            }

            auto collect_duration()
            {
                if (duration.has_value())
                {
                    auto now = rpp::schedulers::clock_type::now();
                    while (!values.empty() && (now - values.front().second > duration.value()))
                    {
                        values.pop_front();
                    }
                    return now;
                }
                return rpp::schedulers::clock_type::time_point{};
            }

            void collect_bound()
            {
                if (count.has_value())
                {
                    if (values.size() == count.value())
                    {
                        values.pop_front();
                    }
                }
            }

            template<typename T>
            void collect(T&& v)
            {
                std::unique_lock lock{list_mutex};
                collect_bound();
                const auto time_point = collect_duration();

                values.emplace_back(std::forward<T>(v), time_point);
            }

            std::optional<size_t>                    count;
            std::optional<rpp::schedulers::duration> duration;

            std::list<std::pair<Type, rpp::schedulers::clock_type::time_point>> values{};

            std::mutex list_mutex{};
            std::mutex serialized_mutex{};
        };

        struct observer_strategy
        {
            std::shared_ptr<replay_state> state;

            template<typename T>
            void collect_and_on_next(T&& v) const
                requires Serialized
            {
                state->collect(std::forward<T>(v));

                std::unique_lock lock{state->serialized_mutex};
                state->on_next(state->values.back().first);
            }

            template<typename T>
            void collect_and_on_next(T&& v) const
            {
                state->collect(std::forward<T>(v));
                state->on_next(state->values.back().first);
            }

            void set_upstream(const disposable_wrapper& d) const noexcept { state->add(d); }

            bool is_disposed() const noexcept
            {
                return state->is_disposed();
            }

            void on_next(Type&& v) const
            {
                collect_and_on_next(std::move(v));
            }

            void on_next(const Type& v) const
            {
                collect_and_on_next(v);
            }

            void on_error(const std::exception_ptr& err) const
                requires Serialized
            {
                std::unique_lock lock{state->serialized_mutex};
                state->on_error(err);
            }

            void on_error(const std::exception_ptr& err) const
            {
                state->on_error(err);
            }

            void on_completed() const
                requires Serialized
            {
                std::unique_lock lock{state->serialized_mutex};
                state->on_completed();
            }

            void on_completed() const
            {
                state->on_completed();
            }
        };

    public:
        replay_strategy()
            : m_state(disposable_wrapper_impl<replay_state>::make(std::nullopt, std::nullopt))
        {
        }

        replay_strategy(size_t count)
            : m_state{disposable_wrapper_impl<replay_state>::make(std::max<size_t>(1, count), std::nullopt)}
        {
        }

        replay_strategy(size_t count, rpp::schedulers::duration duration)
            : m_state{disposable_wrapper_impl<replay_state>::make(std::max<size_t>(1, count), duration)}
        {
        }

        using preferred_disposable_strategy = rpp::details::observers::none_disposable_strategy;

        auto get_observer() const
        {
            return rpp::observer<Type, observer_strategy>{m_state.lock()};
        }

        template<rpp::constraint::observer_of_type<Type> TObs>
        void on_subscribe(TObs&& observer) const
        {
            if (const auto locked = m_state.lock())
            {
                {
                    std::unique_lock lock{locked->list_mutex};
                    locked->collect_duration();
                    for (const auto& value : locked->values)
                    {
                        observer.on_next(value.first);
                    }
                }
                locked->on_subscribe(std::forward<TObs>(observer));
            }
        }

        rpp::disposable_wrapper get_disposable() const
        {
            return m_state;
        }

    private:
        disposable_wrapper_impl<replay_state> m_state;
    };
} // namespace rpp::subjects::details

namespace rpp::subjects
{
    /**
     * @brief Same as rpp::subjects::publish_subject but send all earlier emitted values to any new observers.
     *
     * @param count maximum element count of the replay buffer (optional)
     * @param duration maximum time length the replay buffer (optional)
     *
     * @tparam Type value provided by this subject
     *
     * @ingroup subjects
     * @see https://reactivex.io/documentation/subject.html
     */
    template<rpp::constraint::decayed_type Type>
    class replay_subject final : public details::base_subject<Type, details::replay_strategy<Type, false>>
    {
    public:
        using details::base_subject<Type, details::replay_strategy<Type, false>>::base_subject;
    };

    /**
     * @brief Same as rpp::subjects::replay_subject but on_next/on_error/on_completed calls are serialized via mutex.
     *
     * @ingroup subjects
     * @see https://reactivex.io/documentation/subject.html
     */
    template<rpp::constraint::decayed_type Type>
    class serialized_replay_subject final : public details::base_subject<Type, details::replay_strategy<Type, true>>
    {
    public:
        using details::base_subject<Type, details::replay_strategy<Type, true>>::base_subject;
    };
} // namespace rpp::subjects