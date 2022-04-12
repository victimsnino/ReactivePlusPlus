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
 * \brief This file contains implementation of `error` functions to create rpp::specific_observable
 *
 * \see https://reactivex.io/documentation/operators/empty-never-throw.html
 **/

#include <rpp/sources/create.h>
#include <rpp/sources/fwd.h>
#include <rpp/utils/constraints.h>

#include <exception>

namespace rpp::observable
{
/**
  * \ingroup observables
  * \brief Creates rpp::specific_observable that emits no items and terminates with an error
  * \tparam Type type of value to specify observable
  * \param err exception ptr to be sent to subscriber
  *
  * \see https://reactivex.io/documentation/operators/empty-never-throw.html
  */
template<constraint::decayed_type Type>
auto error(const std::exception_ptr& err)
{
    return create<Type>([err](const auto& sub)
    {
        sub.on_error(err);
    });
}
} // namespace rpp::observable
