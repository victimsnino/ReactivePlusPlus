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

#include <rpp/observers/details/base_observer.hpp>

#include <rpp/defs.hpp>
#include <rpp/utils/functors.hpp>
#include <rpp/utils/function_traits.hpp>

#include <exception>
#include <memory>

namespace rpp
{
template<constraint::decayed_type     Type,
         constraint::on_next_fn<Type> OnNext      = utils::empty_function_t<Type>,
         constraint::on_error_fn      OnError     = utils::rethrow_error_t,
         constraint::on_completed_fn  OnCompleted = utils::empty_function_t<>>
class anonymous_observer final : public details::base_observer<Type>
{
    struct internal_copy{};

public:
    template<constraint::decayed_same_as<OnNext>      TOnNext      = utils::empty_function_t<Type>,
             constraint::decayed_same_as<OnError>     TOnError     = utils::rethrow_error_t,
             constraint::decayed_same_as<OnCompleted> TOnCompleted = utils::empty_function_t<>>
    anonymous_observer(TOnNext&& on_next = {}, TOnError&& on_error = {}, TOnCompleted&& on_completed = {})
        : m_storage{std::forward<TOnNext>(on_next),
                    std::forward<TOnError>(on_error),
                    std::forward<TOnCompleted>(on_completed)} {}

    template<constraint::decayed_same_as<OnNext>      TOnNext,
             constraint::decayed_same_as<OnCompleted> TOnCompleted>
    anonymous_observer(TOnNext&& on_next, TOnCompleted&& on_completed)
        : m_storage{std::forward<TOnNext>(on_next),
                    utils::rethrow_error_t{},
                    std::forward<TOnCompleted>(on_completed)} {}

    anonymous_observer(const anonymous_observer&)     = delete;
    anonymous_observer(anonymous_observer&&) noexcept = default;

    anonymous_observer(internal_copy, const anonymous_observer& other)
        : details::base_observer<Type>{typename details::base_observer<Type>::internal_copy{}, other}
        , m_storage{other.m_storage} {}

    anonymous_observer(internal_copy, anonymous_observer&& other) noexcept
        : details::base_observer<Type>{typename details::base_observer<Type>::internal_copy{}, other}
        , m_storage{std::move(other.m_storage)} {}

    dynamic_observer<Type> as_dynamic() const & final
    {
        return {std::make_shared<anonymous_observer>(internal_copy{}, *this)};
    }

    dynamic_observer<Type> as_dynamic() && final
    {
        return {std::make_shared<anonymous_observer>(internal_copy{}, std::move(*this))};
    }

private:
    void on_next_impl(const Type& v) const final
    {
        m_storage.m_on_next(v);
    }

    void on_next_impl(Type&& v) const final
    {
        m_storage.m_on_next(std::move(v));
    }

    void on_error_impl(const std::exception_ptr& err) const final
    {
        m_storage.m_on_err(err);
    }

    void on_completed_impl() const final
    {
        m_storage.m_on_completed();
    }

private:
    // use separate "storage" due to [[no_unique]] possible issues with vtables
    struct storage
    {
        RPP_NO_UNIQUE_ADDRESS OnNext      m_on_next;
        RPP_NO_UNIQUE_ADDRESS OnError     m_on_err;
        RPP_NO_UNIQUE_ADDRESS OnCompleted m_on_completed;
    };

    storage m_storage;
};

template<typename OnNext>
anonymous_observer(OnNext) -> anonymous_observer<utils::decayed_function_argument_t<OnNext>, OnNext>;

template<typename OnNext, constraint::on_error_fn OnError, typename ...Args>
anonymous_observer(OnNext, OnError, Args...) -> anonymous_observer<utils::decayed_function_argument_t<OnNext>, OnNext, OnError, Args...>;

template<typename OnNext, constraint::on_completed_fn OnCompleted>
anonymous_observer(OnNext, OnCompleted) -> anonymous_observer<utils::decayed_function_argument_t<OnNext>, OnNext, utils::rethrow_error_t, OnCompleted>;
} // namespace rpp
