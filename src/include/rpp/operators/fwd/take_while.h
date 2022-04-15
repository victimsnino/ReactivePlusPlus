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

#include <rpp/observables/member_overload.h>

namespace rpp::details
{
struct take_while_tag;
}
namespace rpp::operators
{
/**
 * \copydoc rpp::details::member_overload::take_while
 */
template<typename Predicate>
auto take_while(Predicate&& predicate) requires details::is_header_included<details::take_while_tag, Predicate>;
} // namespace rpp::operators

namespace rpp::details
{
template<constraint::decayed_type Type, typename SpecificObservable>
struct member_overload<Type, SpecificObservable, take_while_tag>
{
    /**
     * \brief sends items provided by observable while satisfies predicate. When condition becomes false -> terminates
     *
     * Example:
     * \snippet take_while.cpp take_while
     *
     * \see https://reactivex.io/documentation/operators/takewhile.html
     *
     * \return new specific_observable with the take_while operator as most recent operator.
     * \warning #include <rpp/operators/take_while.h>
     * \ingroup operators
     */
    template<std::predicate<const Type&> Predicate>
    auto take_while(Predicate&& predicate) const& requires is_header_included<take_while_tag, Predicate>
    {
        return static_cast<const SpecificObservable*>(this)->template lift<Type>(take_while_impl(std::forward<Predicate>(predicate)));
    }

    template<std::predicate<const Type&> Predicate>
    auto take_while(Predicate&& predicate) && requires is_header_included<take_while_tag, Predicate>
    {
        return std::move(*static_cast<SpecificObservable*>(this)).template lift<Type>(take_while_impl(std::forward<Predicate>(predicate)));
    }

private:
    template<std::predicate<const Type&> Predicate>
    static auto take_while_impl(Predicate&& predicate);
};
} // namespace rpp::details
