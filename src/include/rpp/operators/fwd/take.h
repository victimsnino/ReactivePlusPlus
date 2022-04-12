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
struct take_tag;
}

namespace rpp::operators
{
/**
 * \copydoc rpp::details::member_overload::take
 */
template<typename...Args>
auto take(size_t count) requires details::is_header_included<details::take_tag, Args...>;
} // namespace rpp::operators

namespace rpp::details
{
template<constraint::decayed_type Type, typename SpecificObservable>
struct member_overload<Type, SpecificObservable, take_tag>
{
    /**
     * \brief emit only first Count items provided by observable
     *
     * Example:
     * \code
     * observable.take(10);
     * \endcode
     *
     * \see https://reactivex.io/documentation/operators/take.html
     *
     * \return new specific_observable with the Take operator as most recent operator.
     * \warning #include <rpp/operators/take.h>
     * \ingroup operators
     */
    template<typename...Args>
    auto take(size_t count) const & requires is_header_included<take_tag, Args...>
    {
        return take_impl(*static_cast<const SpecificObservable*>(this), count);
    }

    template<typename...Args>
    auto take(size_t count) && requires is_header_included<take_tag, Args...>
    {
        return take_impl(*static_cast<const SpecificObservable*>(this), count);
    }

private:
    template<constraint::decayed_same_as<SpecificObservable> TObs>
    static auto take_impl(TObs&& _this, size_t count);
};
} // namespace rpp::details
