#pragma once

#include <rpp/observables/details/member_overload.hpp>
#include <rpp/observables/constraints.hpp>
#include <rpp/schedulers/constraints.hpp>


namespace rpp::details
{
struct subscribe_on_tag;
}

namespace rpp::operators
{
/**
 * \copydoc rpp::details::member_overload::subscribe_on
 */
template<schedulers::constraint::scheduler TScheduler>
auto subscribe_on(const TScheduler& scheduler) requires details::is_header_included<details::subscribe_on_tag, TScheduler>
{
    return [scheduler]<constraint::observable TObservable>(TObservable&& observable)
    {
        return std::forward<TObservable>(observable).subscribe_on(scheduler);
    };
}
} // namespace rpp::operators

namespace rpp::details
{
template<constraint::decayed_type Type, constraint::observable_of_type<Type> TObs, schedulers::constraint::scheduler TScheduler>
auto subscribe_on_impl(TObs&& obs, TScheduler&& scheduler);

template<constraint::decayed_type Type, typename SpecificObservable>
struct member_overload<Type, SpecificObservable, subscribe_on_tag>
{
    /**
    * \brief forces observable to operate in provided scheduler (on_subscribe called via scheduling)
    *
    * Example:
    * \snippet subscribe_on.cpp subscribe_on
    *
    * \see https://reactivex.io/documentation/operators/subscribeon.html
    *
    * \return new specific_observable with the subscribe_on operator as most recent operator.
    * \warning #include <rpp/operators/subscribe_on.h>
    * \ingroup operators
    */
    template<schedulers::constraint::scheduler TScheduler>
    auto subscribe_on(const TScheduler& scheduler) const & requires is_header_included<subscribe_on_tag, TScheduler>
    {
        return subscribe_on_impl<Type>(*static_cast<const SpecificObservable*>(this), scheduler);
    }
    
    template<schedulers::constraint::scheduler TScheduler>
    auto subscribe_on(const TScheduler& scheduler) && requires is_header_included<subscribe_on_tag, TScheduler>
    {
        return subscribe_on_impl<Type>(std::move(*static_cast<SpecificObservable*>(this)), scheduler);
    }
};
} // namespace rpp::details
