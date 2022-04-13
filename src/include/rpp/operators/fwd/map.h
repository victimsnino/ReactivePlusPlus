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
struct map_tag;
}

namespace rpp::operators
{
/**
 * \copydoc rpp::details::member_overload::map
 */
template<typename Callable>
auto map(Callable&& callable) requires details::is_header_included<details::map_tag, Callable>;
} // namespace rpp::operators

namespace rpp::details
{
template<constraint::decayed_type Type, typename SpecificObservable>
struct member_overload<Type, SpecificObservable, map_tag>
{
    /**
     * \brief transform the items emitted by an Observable by applying a function to each item
     *
     * \details The Map operator applies a function of your choosing to each item emitted by the source Observable, and returns an Observable that emits the results of these function applications.
     *
     * The Map operator can keep same type of value or change it to some another type.
     *
     * Example with same type:
     * \snippet map.cpp Same type
     *
     * Example with changed type:
     * \snippet map.cpp Changed type
     *
     * \see https://reactivex.io/documentation/operators/map.html
     *
     * \tparam Callable type of callable used to provide this transformation
     * \return new specific_observable with the Map operator as most recent operator.
     * \warning #include <rpp/operators/map.h>
     * \ingroup operators
     */
    template<std::invocable<Type> Callable>
    auto map(Callable&& callable) const & requires is_header_included<map_tag, Callable>
    {
        return map_impl(*static_cast<const SpecificObservable*>(this), std::forward<Callable>(callable));
    }

    template<std::invocable<Type> Callable>
    auto map(Callable&& callable) && requires is_header_included<map_tag, Callable>
    {
        return map_impl(std::move(*static_cast<SpecificObservable*>(this)), std::forward<Callable>(callable));
    }

private:
    template<constraint::decayed_same_as<SpecificObservable> TObs, std::invocable<Type> Callable>
    static auto map_impl(TObs&& _this, Callable&& callable);
};
} // namespace rpp::details
