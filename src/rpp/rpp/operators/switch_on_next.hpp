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
#include <rpp/operators/details/utils.hpp>

#include <rpp/defs.hpp>
#include <rpp/operators/details/strategy.hpp>
#include <rpp/disposables/refcount_disposable.hpp>


namespace rpp::operators::details
{
template <rpp::constraint::observer TObserver>
class switch_on_next_state_t final : public refcount_disposable {
public:

  template <rpp::constraint::decayed_same_as<TObserver> TObs>
  switch_on_next_state_t(TObs &&obs)
      : m_observer_with_mutex{std::forward<TObs>(obs)} {}

  pointer_under_lock<TObserver> get_observer() 
  {
    return pointer_under_lock{m_observer_with_mutex};
  }

private:
  value_with_mutex<TObserver> m_observer_with_mutex{};
};

template<rpp::constraint::observer TObserver>
class switch_on_next_observer_strategy
{
public:
    using preferred_disposable_strategy = rpp::details::observers::none_disposable_strategy;

    template<rpp::constraint::decayed_same_as<TObserver> TObs> 
    switch_on_next_observer_strategy(TObs&& obs)
        : m_state{std::make_shared<switch_on_next_state_t<TObserver>>(std::forward<TObs>(obs))}
    {
        m_state->get_observer()->set_upstream(rpp::disposable_wrapper::from_weak(m_state->get_underlying()));
    }

    template<typename T>
    void on_next(T&& v) const
    {
    }

    void on_error(const std::exception_ptr& err) const 
    { 
        m_state->get_underlying()->dispose(); 
        m_state->get_observer()->on_error(err); 
    }

    void on_completed() const 
    {
        m_state->dispose();
        if (m_state->is_disposed_underlying())
            m_state->get_observer()->on_completed();
    }

    void set_upstream(const disposable_wrapper& d) { m_state->add(d); }

    bool is_disposed() const { return m_state->is_disposed() || m_state->is_disposed_underlying(); }
private:
    std::shared_ptr<switch_on_next_state_t<TObserver>> m_state; 
};

struct switch_on_next_t : public operators::details::operator_observable_strategy<switch_on_next_observer_strategy>
{
    template<rpp::constraint::observable T>
    using result_value = rpp::utils::extract_observable_type_t<T>;

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