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

#include <rpp/defs.hpp>
#include <rpp/operators/details/strategy.hpp>
#include <rpp/disposables/refcount_disposable.hpp>
#include <rpp/operators/details/forwarding_subject.hpp>

#include <cstddef>

namespace rpp
{
template<constraint::decayed_type Type>
using windowed_observable = decltype(std::declval<rpp::operators::details::forwarding_subject<Type>>().get_observable());
}

namespace rpp::operators::details
{
template<rpp::constraint::observer TObserver, typename TClosingsSelectorFn>
struct window_toggle_state
{
    using Observable = rpp::utils::extract_observer_type_t<TObserver>;
    using value_type = rpp::utils::extract_observable_type_t<Observable>;
    using Subject = forwarding_subject<value_type>;

    static_assert(std::same_as<Observable, decltype(std::declval<Subject>().get_observable())>);

    RPP_NO_UNIQUE_ADDRESS TObserver           observer;
    RPP_NO_UNIQUE_ADDRESS TClosingsSelectorFn closings;

    std::mutex                                                            mutex{};
    mutable std::vector<decltype(std::declval<Subject>().get_observer())> observers{};
};

template<rpp::constraint::observer TObserver, typename TClosingsSelectorFn>
struct window_toggle_closing_observer_strategy
{
    using preferred_disposable_strategy = rpp::details::observers::none_disposable_strategy;

    std::shared_ptr<rpp::refcount_disposable>                             disposble;
    std::shared_ptr<window_toggle_state<TObserver, TClosingsSelectorFn>>  state;
    typename window_toggle_state<TObserver, TClosingsSelectorFn>::Subject subj;

    template<typename T>
    void on_next(T&& v) const
    {
        disposble->remove(subj.get_disposable());
        subj.get_observer().on_completed();
        std::lock_guard lock{state->mutex};
        state->observers.erase(state->observers.remove(subj.get_observer()));
    }

    void on_error(const std::exception_ptr& err) const
    {
        disposble->dispose();

        std::lock_guard lock{state->mutex};
        for (const auto& obs : state->observers)
            obs.on_error(err);
        state->observer.on_error(err);
    }
    void on_completed() const {}

    void set_upstream(const disposable_wrapper& d) const { subj.get_disposable().add(d); }
    bool is_disposed() const { return subj.get_disposable().is_disposed(); }
};

template<rpp::constraint::observer TObserver, typename TClosingsSelectorFn>
struct window_toggle_opening_observer_strategy
{
    using preferred_disposable_strategy = rpp::details::observers::none_disposable_strategy;
    
    std::shared_ptr<rpp::refcount_disposable>                            disposble;
    std::shared_ptr<window_toggle_state<TObserver, TClosingsSelectorFn>> state;

    template<typename T>
    void on_next(T&& v) const
    {
        std::lock_guard lock{state->mutex};
        typename window_toggle_state<TObserver, TClosingsSelectorFn>::Subject subject{disposble};
        state->subjects.emplace_back(subject.get_observer());
        state->observer.on_next(subject.get_observable());
        disposble->add(subject.get_disposable());
        state->closings(std::forward<T>(v)).subscribe(subject.get_disposable(), );
    }

    void on_error(const std::exception_ptr& err) const
    {
        disposble->dispose();

        std::lock_guard lock{state->mutex};
        for (const auto& obs : state->observers)
            obs.on_error(err);
        state->observer.on_error(err);
    }
    void on_completed() const {}

    void set_upstream(const disposable_wrapper& d) const { disposble->add(d); }
    bool is_disposed() const { return disposble->is_disposed(); }
};

template<rpp::constraint::observer TObserver, rpp::constraint::observable TOpeningsObservable, typename TClosingsSelectorFn>
    requires rpp::constraint::observable<std::invoke_result_t<TClosingsSelectorFn, rpp::utils::extract_observable_type_t<TOpeningsObservable>>>
class window_toggle_observer_strategy
{
public:
    using preferred_disposable_strategy = rpp::details::observers::none_disposable_strategy;

    window_toggle_observer_strategy(TObserver&& observer, const TOpeningsObservable& openings, const TClosingsSelectorFn& closings)
        : m_state{std::make_shared<window_toggle_state<TObserver, TClosingsSelectorFn>>(std::move(observer), closings)}
    {
        m_state->observer.set_upstream(m_disposble->add_ref());
        m_disposble->add(openings.subscribe_with_disposable());
    }

    void on_next(const auto& v) const
    {
        std::lock_guard lock{m_state->mutex};
        for (const auto& obs : m_state->observers)
            obs.on_next(v);
    }

    void on_error(const std::exception_ptr& err) const
    {
        m_disposble->dispose();

        std::lock_guard lock{m_state->mutex};
        for (const auto& obs : m_state->observers)
            obs.on_error(err);
        m_state->observer.on_error(err);
    }

    void on_completed() const
    {
        m_disposble->dispose();

        std::lock_guard lock{m_state->mutex};
        for (const auto& obs : m_state->observers)
            obs.on_completed();
        m_state->observer.on_completed();
    }

    void set_upstream(const disposable_wrapper& d) const { m_disposble->add(d); }

    bool is_disposed() const { return m_disposble->is_disposed(); }

private:
    std::shared_ptr<rpp::refcount_disposable>                            m_disposble = std::make_shared<refcount_disposable>();
    std::shared_ptr<window_toggle_state<TObserver, TClosingsSelectorFn>> m_state;
};

template<rpp::constraint::observable TOpeningsObservable, typename TClosingsSelectorFn>
    requires rpp::constraint::observable<std::invoke_result_t<TClosingsSelectorFn, rpp::utils::extract_observable_type_t<TOpeningsObservable>>>
struct window_toggle_t : public operators::details::operator_observable_strategy<window_toggle_observer_strategy, TOpeningsObservable, TClosingsSelectorFn>
{
    template<rpp::constraint::decayed_type T>
    using result_value = windowed_observable<T>;

    template<rpp::details::observables::constraint::disposable_strategy Prev>
    using updated_disposable_strategy = rpp::details::observables::fixed_disposable_strategy_selector<1>;
};
}

namespace rpp::operators
{
template<rpp::constraint::observable TOpeningsObservable, typename TClosingsSelectorFn>
    requires rpp::constraint::observable<std::invoke_result_t<TClosingsSelectorFn, rpp::utils::extract_observable_type_t<TOpeningsObservable>>>
auto window_toggle(TOpeningsObservable&& openings, TClosingsSelectorFn&& closings_selector)
{
    return details::window_toggle_t{std::forward<TOpeningsObservable>(openings), std::forward<TClosingsSelectorFn>(closings_selector)};
}
} // namespace rpp::operators