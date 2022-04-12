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

#include <rpp/observables/constraints.h>
#include <rpp/operators/fwd/take_while.h>
#include <rpp/subscribers/constraints.h>
#include <rpp/utils/utilities.h>


#include <utility>

IMPLEMENTATION_FILE(filter_tag);

namespace rpp::operators
{
template<typename Predicate>
auto filter(Predicate&& predicate) requires details::is_header_included<details::filter_tag, Predicate>
{
    return [predicate = std::forward<Predicate>(predicate)]<constraint::observable TObservable>(TObservable && observable)
    {
        return observable.filter(predicate);
    };
}
} // namespace rpp::operators
namespace rpp::details
{
template<constraint::decayed_type Type, typename SpecificObservable>
template<constraint::decayed_same_as<SpecificObservable>TObs, std::predicate<const Type&> Predicate>
auto member_overload<Type, SpecificObservable, filter_tag>::filter_impl(TObs&& _this, Predicate&& predicate)
{
    return std::forward<TObs>(_this).template lift<Type>([predicate = std::forward<Predicate>(predicate)](auto&& value, const constraint::subscriber auto& subscriber)
    {
        if (predicate(utilities::as_const(value)))
            subscriber.on_next(std::forward<decltype(value)>(value));
    });
}
} // namespace rpp::details
