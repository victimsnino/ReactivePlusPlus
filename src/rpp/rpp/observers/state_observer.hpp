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
#include <rpp/defs.hpp>

#include <exception>
#include <tuple>
#include <memory>


namespace rpp::details
{
template<constraint::decayed_type T>
struct type_erased_observer_interface
{
    virtual ~type_erased_observer_interface() = default;

    virtual void on_next(const T& v) const = 0;
    virtual void on_next(T&& v) const = 0;
    virtual void on_error(const std::exception_ptr& err) const = 0;
    virtual void on_completed() const = 0;
    virtual void copy_to(void*) const = 0;
    virtual void move_to(void*) = 0;
};

template<constraint::decayed_type T,
         typename                 OnNext,
         typename                 OnError,
         typename                 OnCompleted,
         constraint::decayed_type ...States>
requires (std::invocable<OnNext, T, States...> && 
          std::invocable<OnError, std::exception_ptr, States...> && 
          std::invocable<OnCompleted, States...>)
class type_erased_observer final : public type_erased_observer_interface<T>
{
public:
    template<typename ...TStates>
        requires (constraint::decayed_same_as<States, TStates> && ...)
    type_erased_observer(std::invocable<T, States...> auto&&                  on_next,
                         std::invocable<std::exception_ptr, States...> auto&& on_error,
                         std::invocable<States...> auto&&                     on_completed,
                         TStates&& ...                                        states)
        : m_data{std::forward<decltype(on_next)>(on_next),
                 std::forward<decltype(on_error)>(on_error),
                 std::forward<decltype(on_completed)>(on_completed),
                 std::forward<TStates>(states)...} {}

    void on_next(const T& v) const override
    {
        std::apply([&v, this](const States& ...states) { m_data.on_next(v, states...); }, m_data.state);
    }

    void on_next(T&& v) const override
    {
        std::apply([&v, this](const States& ...states) { m_data.on_next(std::move(v), states...); }, m_data.state);
    }

    void on_error(const std::exception_ptr& err) const override
    {
        std::apply([&err, this](const States& ...states) { m_data.on_err(err, states...); }, m_data.state);
    }

    void on_completed() const override
    {
        std::apply(m_data.on_completed, m_data.state);
    }

    void copy_to(void* ptr) const override
    {
        std::construct_at(static_cast<type_erased_observer*>(ptr), *this);
    }

    void move_to(void* ptr) override
    {
        std::construct_at(static_cast<type_erased_observer*>(ptr), std::move(*this));
    }

private:
    // struct needed due to [[no_unique_address]] can conflicts with vtbl
    struct data
    {
        data(auto&& on_next, auto&& on_err, auto&& on_completed, auto&& ...state)
            : on_next{std::forward<decltype(on_next)>(on_next)}
            , on_err{std::forward<decltype(on_err)>(on_err)}
            , on_completed{std::forward<decltype(on_completed)>(on_completed)}
            , state{std::forward<decltype(state)>(state)...} {}

        RPP_NO_UNIQUE_ADDRESS OnNext      on_next;
        RPP_NO_UNIQUE_ADDRESS OnError     on_err;
        RPP_NO_UNIQUE_ADDRESS OnCompleted on_completed;

        RPP_NO_UNIQUE_ADDRESS std::tuple<States...> state;
    };

    data m_data{};
};

template<constraint::decayed_type T, size_t StateSize, size_t StateAlignment>
class state_observer_base : public typed_observer_tag<T>
{
public:
    template<typename ...States,
        std::invocable<T, std::decay_t<States>...> OnNext,
        std::invocable<std::exception_ptr, std::decay_t<States>...> OnError,
        std::invocable<std::decay_t<States>...> OnCompleted>
        requires(sizeof(type_erased_observer<T, std::decay_t<OnNext>,
                                             std::decay_t<OnError>,
                                             std::decay_t<OnCompleted>,
                                             std::decay_t<States>...>) == StateSize &&
            alignof(type_erased_observer<T, std::decay_t<OnNext>,
                                         std::decay_t<OnError>,
                                         std::decay_t<OnCompleted>,
                                         std::decay_t<States>...>) == StateAlignment)
    state_observer_base(OnNext&&      on_next,
                        OnError&&     on_error,
                        OnCompleted&& on_completed,
                        States&& ...  states)
    {
        std::construct_at(reinterpret_cast<type_erased_observer<T, std::decay_t<OnNext>,
                                                                std::decay_t<OnError>,
                                                                std::decay_t<OnCompleted>,
                                                                std::decay_t<States>...>*>(m_state),
                          std::forward<OnNext>(on_next),
                          std::forward<OnError>(on_error),
                          std::forward<OnCompleted>(on_completed),
                          std::forward<States>(states)...);
    }

    state_observer_base(const state_observer_base& other)
    {
        other.cast()->copy_to(m_state);
    }

    state_observer_base(state_observer_base&& other) noexcept
    {
        other.cast()->move_to(m_state);
    }

    state_observer_base& operator=(const state_observer_base& other)
    {
        if (this == &other)
            return *this;
        std::destroy_at(cast());
        other.cast()->copy_to(m_state);
        return *this;
    }

    state_observer_base& operator=(state_observer_base&& other) noexcept
    {
        if (this == &other)
            return *this;

        std::destroy_at(cast());
        other.cast()->move_to(m_state);

        return *this;
    }

    ~state_observer_base() noexcept
    {
        std::destroy_at(cast());
    }

    void on_next(const T& v) const
    {
        cast()->on_next(v);
    }

    void on_next(T&& v) const
    {
        cast()->on_next(std::move(v));
    }

    void on_error(const std::exception_ptr& err) const
    {
        cast()->on_error(err);
    }

    void on_completed() const
    {
        cast()->on_completed();
    }

private:
    type_erased_observer_interface<T>* cast()
    {
        return reinterpret_cast<type_erased_observer_interface<T>*>(m_state);
    }

    const type_erased_observer_interface<T>* cast() const
    {
        return reinterpret_cast<const type_erased_observer_interface<T>*>(m_state);
    }

private:
    alignas(StateAlignment) std::byte m_state[StateSize]{};
};


template<constraint::decayed_type T, typename State>
using state_observer_with_state = state_observer_base<T, sizeof(State), alignof(State)>;

template<constraint::decayed_type T,
        typename                 OnNext,
        typename                 OnError,
        typename                 OnCompleted,
        constraint::decayed_type ...States>
    requires (std::invocable<OnNext, T, States...> &&
        std::invocable<OnError, std::exception_ptr, States...> &&
        std::invocable<OnCompleted, States...>)
using state_observer  = state_observer_with_state<T, type_erased_observer<T, OnNext, OnError, OnCompleted, States...>>;

template<typename OnNext,
    typename OnError,
    typename OnCompleted,
    typename ...States>
state_observer_base(OnNext, OnError, OnCompleted, States...) -> state_observer<std::decay_t<utils::function_argument_t<OnNext>>, OnNext, OnError, OnCompleted, States...>;


} // namespace rpp::details
