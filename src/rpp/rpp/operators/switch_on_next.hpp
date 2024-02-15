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
#include <rpp/disposables/refcount_disposable.hpp>
#include <rpp/operators/details/strategy.hpp>
#include <rpp/operators/details/utils.hpp>

namespace rpp::operators::details
{
template<rpp::constraint::observer TObserver>
class switch_on_next_state_t final : public refcount_disposable
{
public:
    template<rpp::constraint::decayed_same_as<TObserver> TObs>
        requires (!rpp::constraint::decayed_same_as<TObs, switch_on_next_state_t<TObserver>>)
    switch_on_next_state_t(TObs&& obs)
        : m_observer_with_mutex{std::forward<TObs>(obs)}
    {
    }

    switch_on_next_state_t(const switch_on_next_state_t&) = delete;
    switch_on_next_state_t(switch_on_next_state_t&&) noexcept = delete;

    pointer_under_lock<TObserver> get_observer()
    {
        return pointer_under_lock{m_observer_with_mutex};
    }

private:
    value_with_mutex<TObserver> m_observer_with_mutex{};
};

template<rpp::constraint::observer TObserver>
class switch_on_next_inner_observer_strategy
{
public:
    using preferred_disposable_strategy = rpp::details::observers::none_disposable_strategy;

    switch_on_next_inner_observer_strategy(const std::shared_ptr<switch_on_next_state_t<TObserver>>& state, const composite_disposable_wrapper& refcounted)
        : m_state{state}
        , m_refcounted{refcounted}
    {
    }

    template<typename T>
    void on_next(T&& v) const
    {
        m_state->get_observer()->on_next(std::forward<T>(v));
    }

    void on_error(const std::exception_ptr& err) const
    {
        m_state->dispose();
        m_state->get_observer()->on_error(err);
    }

    void on_completed() const
    {
        m_refcounted.dispose();
        if (m_state->is_disposed())
            m_state->get_observer()->on_completed();
    }

    void set_upstream(const disposable_wrapper& d) const { m_refcounted.add(d); }

    bool is_disposed() const { return m_refcounted.is_disposed(); }

private:
    std::shared_ptr<switch_on_next_state_t<TObserver>> m_state;
    rpp::composite_disposable_wrapper                  m_refcounted;
};

template<rpp::constraint::observer TObserver>
class switch_on_next_observer_strategy
{
public:
    using preferred_disposable_strategy = rpp::details::observers::none_disposable_strategy;

    switch_on_next_observer_strategy(TObserver&& obs)
        : m_state{init_state(std::move(obs))}
    {
    }

    switch_on_next_observer_strategy(const switch_on_next_observer_strategy&) = delete;
    switch_on_next_observer_strategy(switch_on_next_observer_strategy&&) noexcept = default;

    template<typename T>
    void on_next(T&& v) const
    {
        m_last_refcount.dispose();
        m_last_refcount = m_state->add_ref();
        std::forward<T>(v).subscribe(switch_on_next_inner_observer_strategy<TObserver>{m_state, m_last_refcount});
    }

    void on_error(const std::exception_ptr& err) const
    {
        m_state->dispose();
        m_state->get_observer()->on_error(err);
    }

    void on_completed() const
    {
        m_this_refcount.dispose();
        if (m_state->is_disposed())
            m_state->get_observer()->on_completed();
    }

    void set_upstream(const disposable_wrapper& d) const { m_this_refcount.add(d); }
    bool is_disposed() const { return m_this_refcount.is_disposed(); }
private:
    static std::shared_ptr<switch_on_next_state_t<TObserver>> init_state(TObserver&& observer)
    {
        const auto d = disposable_wrapper_impl<switch_on_next_state_t<TObserver>>::make(std::move(observer));
        auto ptr = d.lock();
        ptr->get_observer()->set_upstream(d.as_weak());
        return ptr;
    }

private:
    std::shared_ptr<switch_on_next_state_t<TObserver>> m_state;
    rpp::composite_disposable_wrapper                  m_this_refcount = m_state->add_ref();
    mutable rpp::composite_disposable_wrapper          m_last_refcount = composite_disposable_wrapper::empty();
};

struct switch_on_next_t  : lift_operator<switch_on_next_t>
{
    template<rpp::constraint::decayed_type T>
    struct operator_traits
    {
        static_assert(rpp::constraint::observable<T>, "T is not observable");

        using result_type = rpp::utils::extract_observable_type_t<T>;

        template<rpp::constraint::observer_of_type<result_type> TObserver>
        using observer_strategy = switch_on_next_observer_strategy<TObserver>;
    };

    template<rpp::details::observables::constraint::disposable_strategy Prev>
    using updated_disposable_strategy = rpp::details::observables::fixed_disposable_strategy_selector<1>;
};
}

namespace rpp::operators
{
inline auto switch_on_next()
{
    return details::switch_on_next_t{};
}
} // namespace rpp::operators