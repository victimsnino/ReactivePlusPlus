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

#include <rpp/observables/constraints.h>
#include <rpp/observables/member_overload.h>
#include <rpp/observables/type_traits.h>

namespace rpp::details
{
struct merge_tag;
}
namespace rpp::operators
{
/**
 * \copydoc rpp::details::member_overload::merge
 */
template<typename ...Args>
auto merge() requires details::is_header_included<details::merge_tag, Args...>;
} // namespace rpp::operators

namespace rpp::details
{
template<constraint::decayed_type Type, typename SpecificObservable>
struct member_overload<Type, SpecificObservable, merge_tag>
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
    * \return new specific_observable with the merge operator as most recent operator.
    * \warning #include <rpp/operators/merge.h>
    * \ingroup operators
    */
    template<typename ...Args>
    auto merge() const& requires (is_header_included<merge_tag, Args...> && rpp::constraint::observable<Type>)
    {
        return static_cast<const SpecificObservable*>(this)->template lift<utils::extract_observable_type_t<Type>>(merge_impl());
    }

    template<typename ...Args>
    auto merge() && requires (is_header_included<merge_tag, Args...>&& rpp::constraint::observable<Type>)
    {
        return std::move(*static_cast<SpecificObservable*>(this)).template lift<utils::extract_observable_type_t<Type>>(merge_impl());
    }

private:
    static auto merge_impl();
};
} // namespace rpp::details
