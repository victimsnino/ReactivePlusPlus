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
    struct filter_tag;
}
namespace rpp::operators
{
/**
 * \copydoc rpp::details::member_overload::filter
 */
template<typename Predicate>
auto filter(Predicate&& predicate) requires details::is_header_included<details::filter_tag, Predicate>;
} // namespace rpp::operators

namespace rpp::details
{
    template<constraint::decayed_type Type, typename SpecificObservable>
    struct member_overload<Type, SpecificObservable, filter_tag>
    {
        /**
         * \brief emit only those items from an Observable that satisfies a predicate
         *
         * \details The Filter operator applies a provided predicate to each item emitted by the Observable, and returns an Observable that emits only items passed provided predicate
         *
         * Example:
         * \code
         * observable.filter([](const int& val)
         *              {
         *                  return val >= 10;
         *              });
         * \endcode
         *
         * \see https://reactivex.io/documentation/operators/filter.html
         *
         * \tparam Predicate type of predicate used to check emitted itemss
         * \return new specific_observable with the Filter operator as most recent operator.
         * \warning #include <rpp/operators/filter.h>
         * \ingroup operators
         */
        template<std::predicate<const Type&> Predicate>
        auto filter(Predicate&& predicate) const& requires is_header_included<filter_tag, Predicate>
        {
            return filter_impl(*static_cast<const SpecificObservable*>(this), std::forward<Predicate>(predicate));
        }

        template<std::predicate<const Type&> Predicate>
        auto filter(Predicate&& predicate) && requires is_header_included<filter_tag, Predicate>
        {
            return filter_impl(*static_cast<const SpecificObservable*>(this), std::forward<Predicate>(predicate));
        }

    private:
        template<constraint::decayed_same_as<SpecificObservable> TObs, std::predicate<const Type&> Predicate>
        static auto filter_impl(TObs&& _this, Predicate&& predicate);
    };
} // namespace rpp::details
