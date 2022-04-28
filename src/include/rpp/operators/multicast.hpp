#pragma once

#include <rpp/observables/constraints.hpp>
#include <rpp/operators/fwd/multicast.hpp>
#include <rpp/observables/connectable_observable.hpp>

IMPLEMENTATION_FILE(multicast_tag);

namespace rpp::operators
{
template<rpp::subjects::constraint::subject TSubject>
auto multicast(TSubject&& subject) requires details::is_header_included<details::multicast_tag, TSubject>
{
    return [subject = std::forward<TSubject>(subject)]<constraint::observable TObservable>(TObservable && observable)
    {
        return std::forward<TObservable>(observable).multicast(subject);
    };
}
} // namespace rpp::operators

namespace rpp::details
{
template<constraint::decayed_type Type, typename SpecificObservable>
template<constraint::decayed_same_as<SpecificObservable> TThis, rpp::subjects::constraint::subject_of_type<Type> TSubject>
auto member_overload<Type, SpecificObservable, multicast_tag>::multicast_impl(TThis&& observable, TSubject&& subject)
{
    return connectable_observable<Type, std::decay_t<TSubject>, SpecificObservable>{std::forward<TThis>(observable), std::forward<TSubject>(subject)};
}
} // namespace rpp::details
