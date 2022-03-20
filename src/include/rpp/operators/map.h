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
#include <rpp/observables/type_traits.h>

#include <utility>

namespace rpp::operators
{
/**
 * \brief transform the items emitted by an Observable by applying a function to each item
 *
 * \details The Map operator applies a function of your choosing to each item emitted by the source Observable, and returns an Observable that emits the results of these function applications.
 *
 * The Map operator can keep same type of value or change it to some another type.
 *
 * Example:
 * \code
 * observable | map([](const int& val)
 *              {
 *                  return std::to_string(val) + " data";
 *              });
 * \endcode
 *
 * \see https://reactivex.io/documentation/operators/map.html
 *
 * \tparam Callable type of callable used to provide this transformation
 * \return new specific_observable with the Map operator as most recent operator.
 */
template<typename Callable>
auto map(Callable&& callable)
{
    return [callable = std::forward<Callable>(callable)]<constraint::observable TObservable>(TObservable&& observable)
    {
        using ObservableType = utils::extract_observable_type_t<TObservable>;
        using NewType = std::invoke_result_t<Callable, ObservableType>;
        return std::forward<TObservable>(observable)
                .template lift<NewType>([callable](auto&& value, const constraint::subscriber auto& subscriber)
                {
                    subscriber.on_next(callable(std::forward<decltype(value)>(value)));
                });
    };
}
} // namespace rpp::operators