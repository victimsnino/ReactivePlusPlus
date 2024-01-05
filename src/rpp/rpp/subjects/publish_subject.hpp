//                   ReactivePlusPlus library
//
//           Copyright Aleksey Loginov 2022 - present.
//  Distributed under the Boost Software License, Version 1.0.
//     (See accompanying file LICENSE_1_0.txt or copy at
//           https://www.boost.org/LICENSE_1_0.txt)
//
//  Project home: https://github.com/victimsnino/ReactivePlusPlus

#pragma once

#include <rpp/subjects/fwd.hpp>

#include <rpp/observers/observer.hpp>
#include <rpp/subjects/details/base_subject.hpp>
#include <rpp/subjects/details/subject_state.hpp>
#include <rpp/disposables/disposable_wrapper.hpp>

#include <memory>

namespace rpp::subjects::details
{
template<rpp::constraint::decayed_type Type>
class publish_strategy
{
    struct observer_strategy
    {
        std::shared_ptr<subject_state<Type>> state{};

        void set_upstream(const disposable_wrapper& d) const noexcept { state->add(d); }

        bool is_disposed() const noexcept { return state->is_disposed(); }

        void on_next(const Type& v) const { state->on_next(v); }

        void on_error(const std::exception_ptr& err) const { state->on_error(err); }

        void on_completed() const { state->on_completed(); }
    };

public:

    using expected_disposable_strategy = rpp::details::observables::deduce_disposable_strategy_t<subject_state<Type>>;

    auto get_observer() const
    {
        return rpp::observer<Type, rpp::details::with_external_disposable<observer_strategy>>{composite_disposable_wrapper{m_state}, observer_strategy{m_state}};
    }

    template<rpp::constraint::observer_of_type<Type> TObs>
    void on_subscribe(TObs&& observer) const
    {
        m_state->on_subscribe(std::forward<TObs>(observer));
    }

    rpp::disposable_wrapper get_disposable() const
    {
        return rpp::disposable_wrapper{m_state};
    }

private:
    std::shared_ptr<subject_state<Type>> m_state = std::make_shared<subject_state<Type>>();
};
} // namespace rpp::subjects::details

namespace rpp::subjects 
{
/**
 * @brief Subject which just multicasts values to observers subscribed on it. It contains two parts: observer and observable at the same time.
 *
 * @details Each observer obtains only values which emitted after corresponding subscribe. on_error/on_completer/unsubscribe cached and provided to new observers if any
 *
 * @warning this subject is not synchronized/serialized! It means, that expected to call callbacks of observer in the serialized way to follow observable contract: "Observables must issue notifications to observers serially (not in parallel).". If you are not sure or need extra serialization, please, use serialized_subject.
 *
 * @tparam Type value provided by this subject
 *
 * @ingroup subjects
 * @see https://reactivex.io/documentation/subject.html
 */
template<rpp::constraint::decayed_type Type>
class publish_subject final : public details::base_subject<Type, details::publish_strategy<Type>>{};
}