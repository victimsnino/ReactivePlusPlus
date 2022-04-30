#pragma once

#include <rpp/observables/constraints.hpp>
#include <rpp/operators/fwd/ref_count.hpp>
#include <rpp/subscribers/constraints.hpp>
#include <rpp/utils/utilities.hpp>
#include <rpp/sources/create.hpp>

IMPLEMENTATION_FILE(ref_count_tag);

namespace rpp::operators
{
template<typename ...Args>
auto ref_count() requires details::is_header_included<details::ref_count_tag, Args...>
{
    return []<constraint::observable TObservable>(TObservable && observable)
    {
        return std::forward<TObservable>(observable).ref_count();
    };
}
} // namespace rpp::operators

namespace rpp::details
{
template<constraint::decayed_type Type, typename SpecificObservable>
template<constraint::decayed_same_as<SpecificObservable> TThis>
auto member_overload<Type, SpecificObservable, ref_count_tag>::ref_count_impl(TThis&& observable)
{
    struct state_t
    {
        size_t                 count_of_active_subs{};
        composite_subscription sub = composite_subscription::empty();
        std::mutex             mutex{};
    };
    return source::create<Type>([observable = std::forward<TThis>(observable), state = std::make_shared<state_t>()](const constraint::subscriber_of_type<Type> auto& subscriber)
    {
        {
            std::lock_guard lock{ state->mutex };
            const bool      need_to_connect = ++state->count_of_active_subs == 1;
            observable.subscribe(subscriber);
            if (need_to_connect)
                state->sub = observable.connect();
        }

        subscriber.get_subscription().add([state = state]
        {
            std::lock_guard lock{ state->mutex };
            if (--state->count_of_active_subs == 0)
                state->sub.unsubscribe();
        });
    });
}
} // namespace rpp::details
