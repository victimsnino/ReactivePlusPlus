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

#include <rpp/memory_model.h>
#include <rpp/schedulers/constraints.h>
#include <rpp/schedulers/fwd.h>

#include <rpp/observables/fwd.h>

namespace rpp::observable
{
//**************************** CREATE ****************//
template<constraint::decayed_type Type, constraint::on_subscribe_fn<Type> OnSubscribeFn>
auto create(OnSubscribeFn&& on_subscribe);

template<typename OnSubscribeFn>
auto create(OnSubscribeFn&& on_subscribe);

//**************************** EMPTY *****************//
template<constraint::decayed_type Type>
auto empty();

//**************************** NEVER *****************//
template<constraint::decayed_type Type>
auto never();

//**************************** ERROR *****************//
template<constraint::decayed_type Type>
auto error(const std::exception_ptr& err);

//*************************** JUST ********************//
template<memory_model memory_model = memory_model::use_stack, typename T, typename ...Ts>
auto just(const schedulers::constraint::scheduler auto& scheduler, T&& item, Ts&& ...items) requires (constraint::decayed_same_as<T, Ts> && ...);

template<memory_model memory_model = memory_model::use_stack, typename T, typename ...Ts>
auto just(T&& item, Ts&& ...items) requires (constraint::decayed_same_as<T, Ts> && ...);
} // namespace rpp::observable

namespace rpp
{
namespace source = observable;
} // namespace rpp
