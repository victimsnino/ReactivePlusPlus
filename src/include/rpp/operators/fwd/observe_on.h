#pragma once

#include <rpp/observables/member_overload.h>
#include <rpp/schedulers/constraints.h>

namespace rpp::details
{
struct observe_on_tag;
}
namespace rpp::operators
{
/**
 * \copydoc rpp::details::member_overload::observe_on
 */
template<schedulers::constraint::scheduler TScheduler>
auto observe_on(TScheduler&& scheduler) requires details::is_header_included<details::observe_on_tag, TScheduler>;
} // namespace rpp::operators

namespace rpp::details
{
template<constraint::decayed_type Type, typename SpecificObservable>
struct member_overload<Type, SpecificObservable, observe_on_tag>
{
    /**
    * \brief transfer emissions of items to provided scheduler
    *
    * \details after applying this operator all next emissions will be provided via scheduler
    *	
    * Example:
    * \snippet observe_on.cpp observe_on
    *
    * \see https://reactivex.io/documentation/operators/observeon.html
    *
    * \return new specific_observable with the observe_on operator as most recent operator.
    * \warning #include <rpp/operators/observe_on.h>
    * \ingroup operators
    */
    template<schedulers::constraint::scheduler TScheduler>
    auto observe_on(TScheduler&& scheduler) const& requires is_header_included<observe_on_tag, TScheduler>
    {
        return static_cast<const SpecificObservable*>(this)->template lift<Type>(observe_on_impl(std::forward<TScheduler>(scheduler)));
    }

    template<schedulers::constraint::scheduler TScheduler>
    auto observe_on(TScheduler&& scheduler) && requires is_header_included<observe_on_tag, TScheduler>
    {
        return std::move(*static_cast<SpecificObservable*>(this)).template lift<Type>(observe_on_impl(std::forward<TScheduler>(scheduler)));
    }

private:
    template<schedulers::constraint::scheduler TScheduler>
    static auto observe_on_impl(TScheduler&& scheduler);
};
} // namespace rpp::details
