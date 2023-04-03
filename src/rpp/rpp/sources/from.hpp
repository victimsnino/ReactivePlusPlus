//                   ReactivePlusPlus library
//
//           Copyright Aleksey Loginov 2023 - present.
//  Distributed under the Boost Software License, Version 1.0.
//     (See accompanying file LICENSE_1_0.txt or copy at
//           https://www.boost.org/LICENSE_1_0.txt)
//
//  Project home: https://github.com/victimsnino/ReactivePlusPlus

#pragma once

#include <rpp/sources/fwd.hpp>
#include <rpp/observables/base_observable.hpp>
#include <rpp/utils/utils.hpp>

#include <exception>
#include <memory>

namespace rpp::details
{
template<constraint::decayed_type Container>
class shared_container
{
public:
    template<typename ...Ts>
        requires (!constraint::variadic_decayed_same_as<shared_container<Container>, Ts...>)
    shared_container(Ts&&...items)
        // raw "new" call to avoid extra copy/moves for items
        : m_container{new Container{std::forward<Ts>(items)...}} {}

    shared_container(const shared_container&) = default;
    shared_container(shared_container&&) noexcept = default;

    auto begin() const { return std::begin(*m_container); }
    auto end() const { return std::end(*m_container); }

private:
    std::shared_ptr<Container> m_container{};
};

template<memory_model memory_model, constraint::iterable Container, typename ...Ts>
auto pack_to_container(Ts&& ...items)
{
    if constexpr (memory_model == memory_model::use_stack)
        return Container{std::forward<Ts>(items)...};
    else
        return shared_container<Container>{std::forward<Ts>(items)...};
}

template<memory_model memory_model, constraint::decayed_type T, typename ...Ts>
auto pack_variadic(Ts&& ...items)
{
    return pack_to_container<memory_model, std::array<T, sizeof...(Ts)>>(std::forward<Ts>(items)...);
}

template<constraint::decayed_type PackedContainer>
struct from_iterable_strategy
{
    PackedContainer container;

    template<constraint::observer_strategy<utils::iterable_value_t<PackedContainer>> Strategy>
    void subscribe(base_observer<utils::iterable_value_t<PackedContainer>, Strategy>&& observer) const
    {
        try
        {
            for (const auto& v : container)
            {
                if (!observer.is_disposed())
                    observer.on_next(v);
            }

            observer.on_completed();
        }
        catch (...)
        {
            observer.on_error(std::current_exception());
        }
    }
};

template<typename PackedContainer>
auto make_from_iterable_observable(PackedContainer&& container)
{
    return base_observable<utils::iterable_value_t<std::decay_t<PackedContainer>>, details::from_iterable_strategy<std::decay_t<PackedContainer>>>{std::forward<PackedContainer>(container)};
}
}

namespace rpp::source
{
/**
 * @brief Creates observable that emits a items from provided iterable
 *
 * @marble from_iterable
   {
       operator "from_iterable({1,2,3,5})": +-1-2-3-5-|
   }
 *
 * @tparam memory_model rpp::memory_model strategy used to handle provided iterable
 * @param scheduler is scheduler used for scheduling of submissions: next item will be submitted to scheduler when previous one is executed
 * @param iterable container with values which will be flattened
 *
 * @par Examples:
 * @snippet from.cpp from_iterable
 * @snippet from.cpp from_iterable with model
 * @snippet from.cpp from_iterable with scheduler
 *
 * @ingroup creational_operators
 * @see https://reactivex.io/documentation/operators/from.html
 */
template<memory_model memory_model/* = memory_model::use_stack*,*/ /* schedulers::constraint::scheduler TScheduler = schedulers::trampoline*/>
auto from_iterable(constraint::iterable auto&& iterable)
{
    return details::make_from_iterable_observable(details::pack_to_container<memory_model, std::decay_t<decltype(iterable)>>(std::forward<decltype(iterable)>(iterable)));
}
}