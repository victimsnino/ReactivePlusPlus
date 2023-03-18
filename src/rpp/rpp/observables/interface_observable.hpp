//                   ReactivePlusPlus library
//
//           Copyright Aleksey Loginov 2023 - present.
//  Distributed under the Boost Software License, Version 1.0.
//     (See accompanying file LICENSE_1_0.txt or copy at
//           https://www.boost.org/LICENSE_1_0.txt)
//
//  Project home: https://github.com/victimsnino/ReactivePlusPlus

#pragma once

#include <functional>
#include <rpp/observers/details/operator_observer.hpp>
#include <rpp/observers/dynamic_observer.hpp>
#include <rpp/observables/fwd.hpp>
#include <rpp/observers/anonymous_observer.hpp>
#include <rpp/disposables/composite_disposable.hpp>

namespace rpp
{
template<constraint::decayed_type Type>
struct interface_observable
{
    virtual ~interface_observable() noexcept = default;

    composite_disposable subscribe(dynamic_observer<Type> observer) const noexcept
    {
        return subscribe_ret_subscription(std::move(observer));
    }

    composite_disposable subscribe(interface_observer<Type>&& observer) const noexcept
    {
        return subscribe_ret_subscription(std::move(observer));
    }

    template<constraint::on_next_fn<Type> TOnNext      = utils::empty_function_t<Type>,
             constraint::on_error_fn      TOnError     = utils::rethrow_error_t,
             constraint::on_completed_fn  TOnCompleted = utils::empty_function_t<>>
    composite_disposable subscribe(TOnNext&& on_next = {}, TOnError&& on_error = {}, TOnCompleted&& on_completed = {}) const noexcept
    {
        return subscribe(anonymous_observer<Type,
                                            std::decay_t<TOnNext>,
                                            std::decay_t<TOnError>,
                                            std::decay_t<TOnCompleted>>{
                             std::forward<TOnNext>(on_next),
                             std::forward<TOnError>(on_error),
                             std::forward<TOnCompleted>(on_completed)});
    }


    template<constraint::on_next_fn<Type> TOnNext      = utils::empty_function_t<Type>,
             constraint::on_completed_fn  TOnCompleted = utils::empty_function_t<>>
    composite_disposable subscribe(TOnNext&& on_next, TOnCompleted&& on_completed) const noexcept
    {
        return subscribe(std::forward<TOnNext>(on_next), utils::rethrow_error_t{}, std::forward<TOnCompleted>(on_completed));
    }

private:
    composite_disposable subscribe_ret_subscription(interface_observer<Type>&& observer) const noexcept
    {
        auto res = composite_disposable::empty();

        class strategy final : public details::forwarding_on_next_strategy
        {
        public:
            strategy(composite_disposable& disposable) : m_disposable{disposable} {}

            void set_resource(const composite_disposable& new_disposable, interface_observer<Type>& obs) {
                obs.set_resource(new_disposable);
                m_disposable.get() = new_disposable;
                m_disposable_original = new_disposable;
            }

            void on_error(const interface_observer<Type>& obs, const std::exception_ptr& err) const noexcept
            {
                obs.on_error(err);
                m_disposable_original.dispose();
            }

            void on_completed(const interface_observer<Type>& obs) const noexcept
            {
                obs.on_completed();
                m_disposable_original.dispose();
            }

        private:
            std::reference_wrapper<composite_disposable> m_disposable;
            mutable composite_disposable m_disposable_original{m_disposable};
        };

        subscribe_impl(details::operator_observer<Type, Type, strategy>{strategy{res}, std::ref(observer)});

        return res;
    }

protected:
    virtual void subscribe_impl(interface_observer<Type>&& observer) const noexcept = 0;
};
} // namespace rpp