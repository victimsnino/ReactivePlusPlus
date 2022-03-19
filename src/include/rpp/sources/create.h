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
 * \brief This file contains implementation of `create` function to create rpp::specific_observable with OnSubscribe callback
 *
 * \see https://reactivex.io/documentation/operators/create.html
 **/

#include <type_traits>
#include <rpp/observables/specific_observable.h>
#include <rpp/utils/type_traits.h>

namespace rpp::observable
{
/**
 * \ingroup observables
 * \brief Creates rpp::specific_observable with passed action as OnSubscribe
 * \tparam Type manually specified type of value provided by this observable
 * \tparam OnSubscribeFn action called after subscription on this observable
 * \return rpp::specific_observable with passed action
 *
 * \see https://reactivex.io/documentation/operators/create.html
 */
template<typename Type, typename OnSubscribeFn>
auto create(OnSubscribeFn&& on_subscribe)
{
    return specific_observable<Type, std::decay_t<OnSubscribeFn>>{std::forward<OnSubscribeFn>(on_subscribe)};
}

/**
 * \ingroup observables
 * \brief Creates specific_observable with passed action as OnSubscribe
 * \tparam OnSubscribeFn action called after subscription on this observable
 * \return specific_observable with passed action
 *
 * \see https://reactivex.io/documentation/operators/create.html
 */
template<typename OnSubscribeFn>
auto create(OnSubscribeFn&& on_subscribe)
{
    return create<utils::extract_subscriber_type_t<utils::function_argument_t<OnSubscribeFn>>>(std::forward<OnSubscribeFn>(on_subscribe));
}
} // namespace rpp::observable