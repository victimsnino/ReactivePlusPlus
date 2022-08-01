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

#include <rpp/observers/fwd.hpp>
#include <rpp/utils/function_traits.hpp>            // extract argument type

#include <exception>
#include <tuple>

namespace rpp::details
{
/**
 * \brief Special type of specific_observer which has some state which this observer stores and pass to each callback. Used for storing subscriber without extra copies
 */
template<constraint::decayed_type T,
         typename                 OnNext,
         typename                 OnError,
         typename                 OnCompleted,
         constraint::decayed_type ...States>
requires (std::invocable<OnNext, T, States...> && 
          std::invocable<OnError, std::exception_ptr, States...> && 
          std::invocable<OnCompleted, States...>)
class state_observer : public typed_observer<T>
{
public:
    template<typename ...TStates>
        requires (constraint::decayed_same_as<States, TStates> && ...)
    state_observer(std::invocable<T, States...> auto&&                  on_next,
                   std::invocable<std::exception_ptr, States...> auto&& on_error,
                   std::invocable<States...> auto&&                     on_completed,
                   TStates&&                                            ...states)
        : m_on_next{std::forward<decltype(on_next)>(on_next)}
        , m_on_err{std::forward<decltype(on_error)>(on_error)}
        , m_on_completed{std::forward<decltype(on_completed)>(on_completed)}
        , m_state{std::forward<TStates>(states)...} {}

    /**
     * \brief Observable calls this methods to notify observer about new value.
     *
     * \note obtains value by const-reference to original object.
     */
    void on_next(const T& v) const
    {
        std::apply([&](const States& ...states) { m_on_next(v, states...); }, m_state);
    }

    /**
     * \brief Observable calls this methods to notify observer about new value.
     *
     * \note obtains value by rvalue-reference to original object
     */
    void on_next(T&& v) const
    {
        std::apply([&](const States& ...states) { m_on_next(std::move(v), states...); }, m_state);
    }

    /**
     * \brief Observable calls this method to notify observer about some error during generation next data.
     * \warning Obtaining this call means no any further on_next or on_completed calls
     * \param err details of error
     */
    void on_error(const std::exception_ptr& err) const
    {
        std::apply([&](const States& ...states) { m_on_err(err, states...); }, m_state);
    }

    /**
     * \brief Observable calls this method to notify observer about finish of work.
     * \warning Obtaining this call means no any further on_next calls
     */
    void on_completed() const
    {
        std::apply(m_on_completed, m_state);
    }

private:
    [[msvc::no_unique_address]] OnNext                m_on_next;
    [[msvc::no_unique_address]] OnError               m_on_err;
    [[msvc::no_unique_address]] OnCompleted           m_on_completed;

    [[msvc::no_unique_address]] std::tuple<States...> m_state;
};

template<typename TOnNext, typename ...Args>
state_observer(TOnNext, Args...)->state_observer<std::decay_t<utils::function_argument_t<TOnNext>>, TOnNext, Args...>;

} // namespace rpp::details