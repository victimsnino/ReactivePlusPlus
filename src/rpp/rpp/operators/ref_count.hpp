#pragma once

#include <rpp/observables/connectable_observable.hpp>

namespace rpp::operators::details
{
    struct ref_count_t
    {
        template<rpp::constraint::observable OriginalObservable, rpp::constraint::subject Subject>
        auto operator()(const connectable_observable<OriginalObservable, Subject>& observable) const
        {
            return observable.ref_count();
        }
    };
}

namespace rpp::operators
{
/**
* @brief Forces rpp::connectable_observable to behave like common observable
* @details Connects rpp::connectable_observable on the first subscription and unsubscribes on last unsubscription
*
* @par Example
* @snippet ref_count.cpp ref_count_operator
*
* @ingroup connectable_operators
* @see https://reactivex.io/documentation/operators/refcount.html
*/
inline auto ref_count()
{
    return rpp::operators::details::ref_count_t{};
}
}