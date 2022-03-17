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

/** \file
 *  \brief This file contains implementation of different function for creation of observables from scratch (sources of streams)
 *
 *  \defgroup observables Observables
 *  \brief Observable is the source of any Reactive Stream. Observable provides ability to subscribe observer on some events.
 *  \see https://reactivex.io/documentation/observable.html
 **/

#include <rpp/observables/specific_observable.h>

namespace rpp::observable
{
/**
 * \brief Creates specific_observable with passed action as OnSubscribe
 * \tparam Type manually specified typed of values provided by this observable
 * \tparam OnSubscribeFn action called after subscription on this observable
 * \return specific_observable with passed action
 */
template<typename Type, typename OnSubscribeFn>
specific_observable<Type, std::remove_const_t<std::remove_reference_t<OnSubscribeFn>>> create(OnSubscribeFn&& on_subscribe)
{
    return {std::forward<OnSubscribeFn>(on_subscribe)};
}

/**
 * \brief Creates specific_observable with passed action as OnSubscribe
 * \tparam OnSubscribeFn action called after subscription on this observable
 * \tparam Type of values for observable deduced by argument of passed action (argument -> subscriber of some type -> type)
 * \return specific_observable with passed action
 */
template<typename OnSubscribeFn, typename Type = utils::extract_subscriber_type_t<utils::function_argument_t<OnSubscribeFn>>>
auto create(OnSubscribeFn&& on_subscribe)
{
    return create<Type>(std::forward<OnSubscribeFn>(on_subscribe));
}
} // namespace rpp::observable