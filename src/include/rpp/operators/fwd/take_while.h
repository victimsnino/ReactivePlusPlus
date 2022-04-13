// MIT License
// 
// Copyright (c) 2022 Aleksey Loginov
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

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
