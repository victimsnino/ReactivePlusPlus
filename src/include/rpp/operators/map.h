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

/**
 * \file
 *
 * \brief This file contains implementation of operator "map"
 */

#include <rpp/observables/constraints.h>
#include <rpp/subscribers/constraints.h>
#include <rpp/observables/type_traits.h>
#include <rpp/operators/fwd/map.h>

#include <utility>

namespace rpp::operators
{
template<typename Callable>
auto map(Callable&& callable)
{
    return [callable = std::forward<Callable>(callable)]<constraint::observable TObservable>(TObservable&& observable)
    {
        return observable.map(callable);
    };
}
} // namespace rpp::operators

namespace rpp::details
{
template<typename ...AN>
struct operator_declaration<map_tag, AN...>
{
    static std::true_type header_included();
};

template<constraint::decayed_type Type, typename SpecificObservable>
template<constraint::decayed_same_as<SpecificObservable> TObs, std::invocable<Type> Callable>
auto member_overload<Type, SpecificObservable, map_tag>::map_impl(TObs&& _this, Callable&& callable)
{
    using NewType = std::invoke_result_t<Callable, Type>;

    return std::forward<TObs>(_this)
            .template lift<NewType>([callable = std::forward<Callable>(callable)](auto&& value, const constraint::subscriber auto& subscriber)
            {
                subscriber.on_next(callable(std::forward<decltype(value)>(value)));
            });
}
} // namespace rpp::details
