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

#include <rpp/operators/fwd.hpp>

#include <rpp/operators/details/strategy.hpp>
#include <rpp/operators/details/utils.hpp>

namespace rpp::operators::details
{
    template<rpp::constraint::observer TObserver, rpp::constraint::observable TFallbackObservable, rpp::details::disposables::constraint::disposable_container Container>
    class timeout_disposable final : public rpp::composite_disposable_impl<Container>
    {
    public:
        struct observer_with_timeout
        {
            TObserver observer;
            rpp::schedulers::time_point timeout;
        };

        timeout_disposable(TObserver&& observer, rpp::schedulers::duration period, const TFallbackObservable& fallback, rpp::schedulers::time_point timeout)
            : m_observer_with_timeout{std::move(observer), timeout}
            , m_period{period}
            , m_fallback{fallback}
        {
        }
        pointer_under_lock<observer_with_timeout> get_observer_with_timeout_under_lock() { return pointer_under_lock{m_observer_with_timeout}; }

        const TFallbackObservable& get_fallback() const { return m_fallback; }

    private:
        value_with_mutex<observer_with_timeout>     m_observer_with_timeout;

        const rpp::schedulers::duration m_period;
        const TFallbackObservable       m_fallback;
    };

    template<rpp::constraint::observer TObserver, rpp::constraint::observable TFallbackObservable, rpp::details::disposables::constraint::disposable_container Container>
    struct timeout_disposable_wrapper
    {
        std::shared_ptr<timeout_disposable<TObserver, TFallbackObservable, Container>> disposable;

        bool is_disposed() const { return disposable->is_disposed(); }

        void on_error(const std::exception_ptr& err) const
        {
            disposable->dispose();
            disposable->get_observer_with_timeout_under_lock()->observer.on_error(err);
        }
    };

    template<rpp::constraint::observer TObserver, rpp::constraint::observable TFallbackObservable, rpp::details::disposables::constraint::disposable_container Container, rpp::schedulers::constraint::scheduler TScheduler>
    struct timeout_observer_strategy
    {
        using preferred_disposable_strategy = rpp::details::observers::none_disposable_strategy;

        std::shared_ptr<timeout_disposable<TObserver, TFallbackObservable, Container>> disposable;

        void set_upstream(const rpp::disposable_wrapper& d) const
        {
            disposable->add(d);
        }

        bool is_disposed() const
        {
            return disposable->is_disposed();
        }

        template<typename T>
        void on_next(T&& v) const
        {
            const auto obs_with_timeout = disposable->get_observer_with_timeout_under_lock();
            obs_with_timeout->observer.on_next(std::forward<T>(v));
            obs_with_timeout->timeout = disposable->scheduler.now() + disposable->m_period;
        }

        void on_error(const std::exception_ptr& err) const noexcept
        {
            const auto obs_with_timeout = disposable->get_observer_with_timeout_under_lock();
            if (disposable->is_disposed())
                return;

            disposable->dispose();
            obs_with_timeout->observer.on_error(err);
        }

        void on_completed() const noexcept
        {
            const auto obs_with_timeout = disposable->get_observer_with_timeout_under_lock();
            if (disposable->is_disposed())
                return;

            disposable->dispose();
            obs_with_timeout->observer.on_completed();
        }
    };

    template<rpp::constraint::observable TFallbackObservable, rpp::schedulers::constraint::scheduler TScheduler>
    struct timeout_t
    {
        template<rpp::constraint::decayed_type T>
        struct operator_traits
        {
            using result_type = T;
        };

        rpp::schedulers::duration                 period;
        RPP_NO_UNIQUE_ADDRESS TFallbackObservable fallback;
        RPP_NO_UNIQUE_ADDRESS TScheduler          scheduler;

        template<rpp::constraint::decayed_type Type, rpp::details::observables::constraint::disposable_strategy DisposableStrategy, rpp::constraint::observer Observer>
        auto lift_with_disposable_strategy(Observer&& observer) const
        {
            using worker_t  = rpp::schedulers::utils::get_worker_t<TScheduler>;
            using container = typename DisposableStrategy::template add<worker_t::is_none_disposable ? 0 : 1>::disposable_container;

            const auto timeout = TScheduler::now() + period;

            const auto disposable = disposable_wrapper_impl<timeout_disposable<std::decay_t<Observer>, TFallbackObservable, container>>::make(std::forward<Observer>(observer), period, fallback, timeout);
            auto       ptr        = disposable.lock();
            ptr->get_observer_with_timeout_under_lock()->observer.set_upstream(disposable.as_weak());

            const auto worker = scheduler.create_worker();
            if constexpr (!rpp::schedulers::utils::get_worker_t<TScheduler>::is_none_disposable)
            {
                if (auto d = worker.get_disposable(); !d.is_disposed())
                    disposable.add(std::move(d));
            }

            worker.schedule(
                timeout,
                [](timeout_disposable_wrapper<std::decay_t<Observer>, TFallbackObservable, container>& handler) -> rpp::schedulers::optional_delay_to {
                    auto locked_obs_with_timeout = handler.disposable.get_observer_with_timeout_under_lock();
                    if (TScheduler::now() < locked_obs_with_timeout->timeout)
                        return rpp::schedulers::delay_to(locked_obs_with_timeout->timeout);

                    handler.disposable->dispose();
                    handler.disposable->get_fallback().subscribe(std::move(locked_obs_with_timeout->observer));
                    return std::nullopt;
                }, ptr);

            return rpp::observer<Type, timeout_observer_strategy<std::decay_t<Observer>, TFallbackObservable, container, TScheduler>>{std::move(ptr)};
        }
    };
} // namespace rpp::operators::details

namespace rpp::operators
{
    template<rpp::constraint::observable TFallbackObservable, rpp::schedulers::constraint::scheduler TScheduler /* = rpp::schedulers::immediate*/>
    auto timeout(rpp::schedulers::duration period, TFallbackObservable&& fallback_observable, const TScheduler& scheduler /* = {}*/)
    {
        return details::timeout_t<std::decay_t<TFallbackObservable>, TScheduler>{period, std::forward<TFallbackObservable>(fallback_observable), scheduler};
    }
} // namespace rpp::operators