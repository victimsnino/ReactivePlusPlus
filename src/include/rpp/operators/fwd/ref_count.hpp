#pragma once

#include <rpp/observables/member_overload.hpp>

namespace rpp::details
{
struct ref_count_tag;
}
namespace rpp::operators
{
/**
 * \copydoc rpp::details::member_overload::ref_count
 */
template<typename ...Args>
auto ref_count() requires details::is_header_included<details::ref_count_tag, Args...>;
} // namespace rpp::operators

namespace rpp::details
{
template<constraint::decayed_type Type, typename SpecificObservable>
struct member_overload<Type, SpecificObservable, ref_count_tag>
{
    /**
    * \brief
    *
    * \details
    *	
    * Example:
    *
    * \see 
    *
    * \return new specific_observable with the ref_count operator as most recent operator.
    * \warning #include <rpp/operators/ref_count.h>
    * \ingroup operators
    */
    template<typename ...Args>
    auto ref_count() const& requires is_header_included<ref_count_tag, Args...>
    {
        return ref_count_impl(*static_cast<const SpecificObservable*>(this));
    }

    template<typename ...Args>
    auto ref_count() && requires is_header_included<ref_count_tag, Args...>
    {
        return ref_count_impl(std::move(*static_cast<SpecificObservable*>(this)));
    }

private:
    template<constraint::decayed_same_as<SpecificObservable> TThis>
    static auto ref_count_impl(TThis&& observable);
};
} // namespace rpp::details
