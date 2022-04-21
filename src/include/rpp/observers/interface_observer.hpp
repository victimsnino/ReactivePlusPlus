//                  ReactivePlusPlus library
//
//          Copyright Aleksey Loginov 2022 - present.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)
//
// Project home: https://github.com/victimsnino/ReactivePlusPlus
//

#pragma once

#include <rpp/observers/fwd.hpp>

#include <exception>


/**
 * \defgroup observers Observers
 * \brief Observer subscribe on observable and obtains values provided by observable.
 */

namespace rpp
{
namespace details
{
struct observer_tag {};
} // namespace details

/**
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
