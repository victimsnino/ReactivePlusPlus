//                  ReactivePlusPlus library
//
//          Copyright Aleksey Loginov 2023 - present.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)
//
// Project home: https://github.com/victimsnino/ReactivePlusPlus
//

#pragma once

#include <rpp/utils/constraints.hpp>
#include <rpp/utils/function_traits.hpp>

namespace rpp::details
{
    template<rpp::constraint::decayed_type TDisposable>
    class auto_dispose_wrapper;
} // namespace rpp::details

namespace rpp
{
    struct interface_disposable;
    struct interface_composite_disposable;

    template<rpp::constraint::decayed_type TDisposable>
    class disposable_wrapper_impl;

    /**
     * @brief Wrapper to keep "simple" disposable. Specialization of rpp::disposable_wrapper_impl
     *
     * @ingroup disposables
     */
    using disposable_wrapper = disposable_wrapper_impl<interface_disposable>;

    /**
     * @brief Wrapper to keep "composite" disposable. Specialization of rpp::disposable_wrapper_impl
     *
     * @ingroup disposables
     */
    using composite_disposable_wrapper = disposable_wrapper_impl<interface_composite_disposable>;
} // namespace rpp

namespace rpp::details::disposables
{
    template<size_t Count>
    class dynamic_disposables_container;

    template<size_t Count>
    class static_disposables_container;

    struct none_disposables_container;

    namespace constraint
    {
        template<typename T>
        concept disposable_container = requires(T& c, const T& const_c, const rpp::disposable_wrapper& d) {
            c.push_back(d);
            const_c.dispose();
            c.clear();
        };
    } // namespace constraint
} // namespace rpp::details::disposables

namespace rpp
{
    class composite_disposable;

    template<rpp::constraint::is_nothrow_invocable Fn>
    class callback_disposable;

    class refcount_disposable;

    template<rpp::constraint::is_nothrow_invocable Fn>
    disposable_wrapper make_callback_disposable(Fn&& invocable);
} // namespace rpp
