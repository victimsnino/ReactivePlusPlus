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

#include <rpp/observers/fwd.h>

#include <exception>

namespace rpp
{
namespace details
{
struct observer_tag {};
} // namespace details

/**
 * \defgroup observers Observers
 * \brief Observer subscribe on observable and obtains values provided by observable.
 *
 *
 * \brief Interface of any observer type. Describes base interface in terms of Reactive Programming
 * \tparam T is type of value handled by this observer
 */
template<constraint::decayed_type T>
struct interface_observer : public details::observer_tag
{
    virtual ~interface_observer() = default;

    /**
     * \brief Observable calls this methods to notify observer about new value.
     *
     * \note obtains value by const-reference to original object.
     */
    virtual void on_next(const T& v) const = 0;

    /**
     * \brief Observable calls this methods to notify observer about new value.
     *
     * \note obtains value by rvalue-reference to original object
     */
    virtual void on_next(T&& v) const = 0;


    /**
     * \brief Observable calls this method to notify observer about some error during generation next data.
     * \warning Obtaining this call means no any further on_next or on_completed calls
     * \param err details of error
     */
    virtual void on_error(const std::exception_ptr& err) const = 0;


    /**
     * \brief Observable calls this method to notify observer about finish of work.
     * \warning Obtaining this call means no any further on_next calls
     */
    virtual void on_completed() const = 0;
};

} // namespace rpp
