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

#include <rpp/observers/interface_observer.hpp> // base class 
#include <rpp/utils/function_traits.hpp>        // extract function args
#include <rpp/utils/functors.hpp>               // default arguments
#include <rpp/observers/state_observer.hpp>     // base class 

namespace rpp
{
/**
 * \brief Observer specified with specific template types of callbacks to avoid extra heap usage.
 *
 * \details It has better performance comparing to rpp::dynamic_observer due to using stack instead of heap as long as possible. It is default return type of all operators
 *
 * \tparam T is type of value handled by this observer
 * \tparam OnNext type of on_next callback
 * \tparam OnError type of on_error callback
 * \tparam OnCompleted type of on_completed callback
 * \ingroup observers
 */
template<constraint::decayed_type T,
         constraint::on_next_fn<T>   OnNext      = utils::empty_function_t<T>,
         constraint::on_error_fn     OnError     = utils::rethrow_error_t,
         constraint::on_completed_fn OnCompleted = utils::empty_function_t<>>
class specific_observer : public details::state_observer<T, OnNext, OnError, OnCompleted>
{
    using base = details::state_observer<T, OnNext, OnError, OnCompleted>;

    using base::base;
public:
    template<constraint::on_next_fn<T>   TOnNext      = utils::empty_function_t<T>,
             constraint::on_error_fn     TOnError     = utils::rethrow_error_t,
             constraint::on_completed_fn TOnCompleted = utils::empty_function_t<>>
    specific_observer(TOnNext&& on_next = {}, TOnError&& on_error = {}, TOnCompleted&& on_completed = {})
        : base{std::forward<TOnNext>(on_next),
               std::forward<TOnError>(on_error),
               std::forward<TOnCompleted>(on_completed)} {}

    specific_observer(constraint::on_next_fn<T> auto&& on_next, constraint::on_completed_fn auto&& on_completed)
        : base{std::forward<decltype(on_next)>(on_next),
               utils::rethrow_error_t{},
               std::forward<decltype(on_completed)>(on_completed)} {}

    /**
     * \brief Converting current rpp::specific_observer to rpp::dynamic_observer alternative with erasing of type (and using heap)
     * \return converted rpp::dynamic_observer
     */
    [[nodiscard]] auto as_dynamic() const& { return dynamic_observer<T>{*this}; }
    [[nodiscard]] auto as_dynamic()&& { return dynamic_observer<T>{std::move(*this)}; }
};

template<typename OnNext>
specific_observer(OnNext) -> specific_observer<utils::decayed_function_argument_t<OnNext>, OnNext>;

template<typename OnNext, constraint::on_error_fn OnError, typename ...Args>
specific_observer(OnNext, OnError, Args...) -> specific_observer<utils::decayed_function_argument_t<OnNext>, OnNext, OnError, Args...>;

template<typename OnNext, constraint::on_completed_fn OnCompleted>
specific_observer(OnNext, OnCompleted) -> specific_observer<utils::decayed_function_argument_t<OnNext>, OnNext, utils::rethrow_error_t, OnCompleted>;

template<typename...Args>
using specific_observer_with_decayed_args = rpp::specific_observer<std::decay_t<Args>...>;

/**
 * \brief Create specific_observer with manually specified Type. In case of type can be deduced from argument of OnNext use direct constructor of rpp::specific_observer
 * \tparam Type manually specific type of observer
 */
template<constraint::decayed_type Type>
auto make_specific_observer() -> specific_observer_with_decayed_args<Type>
{
    return {};
}

template<constraint::decayed_type Type>
auto make_specific_observer(constraint::on_next_fn<Type> auto&& on_next) -> specific_observer_with_decayed_args<Type, decltype(on_next)>
{
    return {std::forward<decltype(on_next)>(on_next)};
}

template<constraint::decayed_type Type>
auto make_specific_observer(constraint::on_next_fn<Type> auto&& on_next,
                            constraint::on_completed_fn auto&&  on_completed) -> specific_observer_with_decayed_args<Type, decltype(on_next), utils::rethrow_error_t, decltype(on_completed)>
{
    return {std::forward<decltype(on_next)>(on_next), std::forward<decltype(on_completed)>(on_completed)};
}

template<constraint::decayed_type Type>
auto make_specific_observer(constraint::on_next_fn<Type> auto&& on_next,
                            constraint::on_error_fn auto&&      on_error) -> specific_observer_with_decayed_args<Type, decltype(on_next), decltype(on_error)>
{
    return {std::forward<decltype(on_next)>(on_next), std::forward<decltype(on_error)>(on_error)};
}

template<constraint::decayed_type Type>
auto make_specific_observer(constraint::on_next_fn<Type> auto&& on_next,
                            constraint::on_error_fn auto&&      on_error,
                            constraint::on_completed_fn auto&&  on_completed) -> specific_observer_with_decayed_args<Type, decltype(on_next), decltype(on_error), decltype(on_completed)>
{
    return {std::forward<decltype(on_next)>(on_next),
            std::forward<decltype(on_error)>(on_error),
            std::forward<decltype(on_completed)>(on_completed)};
}


namespace details
{
    template<constraint::decayed_type Type, typename ...Args>
    using deduce_specific_observer_type_t = decltype(make_specific_observer<Type>(std::declval<Args>()...));
} // namespace details
} // namespace rpp
