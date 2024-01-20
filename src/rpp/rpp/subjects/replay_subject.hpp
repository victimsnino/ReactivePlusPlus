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

namespace rpp::subjects::details
{
template<rpp::constraint::decayed_type Type>
class replay_strategy
{
    struct replay_state
    {
        std::optional<size_t>                    count;
        std::optional<rpp::schedulers::duration> duration;

        std::list<Type>                                    values{};
        std::list<rpp::schedulers::clock_type::time_point> time_points{};

        std::mutex          mutex{};
        subject_state<Type> state{};
    };

    struct observer_strategy
    {
        std::shared_ptr<replay_state> state;

        void set_upstream(const disposable_wrapper& d) const noexcept { state->state.add(d); }

        bool is_disposed() const noexcept
        {
            return state->state.is_disposed();
        }

        void on_next(const Type& v) const
        {
            std::unique_lock lock{state->mutex};
            if (state->count.has_value())
            {
                if (state->values.size() == state->count.value())
                {
                    state->values.pop_front();
                    if (state->duration.has_value())
                    {
                        state->time_points.pop_front();
                    }
                }
            }
            if (state->duration.has_value())
            {
                auto now = rpp::schedulers::clock_type::now();
                while (!state->time_points.empty() && (now - state->time_points.front() > state->duration.value()))
                {
                    state->values.pop_front();
                    state->time_points.pop_front();
                }
                state->time_points.push_back(now);
            }

            state->values.push_back(v);
            state->state.on_next(v);
        }

        void on_error(const std::exception_ptr& err) const
        {
            std::unique_lock lock{state->mutex};
            state->state.on_error(err);
        }

        void on_completed() const
        {
            std::unique_lock lock{state->mutex};
            state->state.on_completed();
        }
    };

public:
    replay_strategy()
        : m_state(std::make_shared<replay_state>(std::nullopt, std::nullopt))
    {
    }

    replay_strategy(size_t count)
        : m_state{std::make_shared<replay_state>(count, std::nullopt)}
    {
    }

    replay_strategy(size_t count, rpp::schedulers::duration duration)
        : m_state{std::make_shared<replay_state>(count, duration)}
    {
    }

    auto get_observer() const
    {
        return rpp::observer<Type, rpp::details::with_external_disposable<observer_strategy>>{
            composite_disposable_wrapper{
                std::shared_ptr<subject_state<Type>>{m_state, &m_state->state}},
            observer_strategy{m_state}};
    }

    template<rpp::constraint::observer_of_type<Type> TObs>
    void on_subscribe(TObs&& observer) const
    {
        {
            std::unique_lock lock{m_state->mutex};
            for (const auto& value : m_state->values)
            {
                observer.on_next(value);
            }
        }
        m_state->state.on_subscribe(std::forward<TObs>(observer));
    }

    rpp::disposable_wrapper get_disposable() const
    {
        return rpp::disposable_wrapper{m_state->state};
    }

private:
    std::shared_ptr<replay_state> m_state = std::make_shared<replay_state>();
};
}

namespace rpp::subjects
{
/**
 * @brief Same as rpp::subjects::serialized_subject but send all earlier emitted values to any new observers.
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
class replay_subject final : public details::base_subject<Type, details::replay_strategy<Type>>
{
public:
    using details::base_subject<Type, details::replay_strategy<Type>>::base_subject;
};
}