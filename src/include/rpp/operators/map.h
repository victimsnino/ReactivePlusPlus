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

#include <utility>

namespace rpp::operators
{
/**
 * \brief map operator describes how to map/transform values of original type to another value.
 *
 * It can change either change type of original value (for example, from double to string) or keep same type but with modified value.
 *
 * Usage example:
 * \code
 * observable
 *      .map([](const int& val)
 *      {
 *          return std::to_string(val) + " data";
 *      });
 * \endcode
 *
 * \tparam Callable type of callable used to provide this transformation
 * \return new specific_observable with map operator as last operator in chain.
 */
template<typename Callable>
auto map(Callable&& callable)
{
    return [callable = std::forward<Callable>(callable)](auto&& observable)
    {
        using TObservable = decltype(observable);
        using ObservableType = utils::extract_observable_type_t<TObservable>;
        using NewType = std::invoke_result_t<Callable, ObservableType>;
        return std::forward<TObservable>(observable).template lift<NewType>([callable](auto&&      value,
                                                                                       const auto& subscriber)
        {
            subscriber.on_next(callable(std::forward<decltype(value)>(value)));
        });
    };
}
} // namespace rpp::operators