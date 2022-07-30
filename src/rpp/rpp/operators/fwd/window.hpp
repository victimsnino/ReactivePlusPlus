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

#include <rpp/observables/constraints.hpp>
#include <rpp/observables/details/member_overload.hpp>

namespace rpp::details
{
struct window_tag;
}

namespace rpp::details
{
template<constraint::decayed_type Type, constraint::observable_of_type<Type> TObs>
auto window_impl(TObs&& obs, size_t window_size);

template<constraint::decayed_type Type, typename SpecificObservable>
struct member_overload<Type, SpecificObservable, window_tag>
{
    /**
     * \brief Subdivide original observable into sub-observables (windowed observables) and emit sub-observables of items instead of original items
     * 
     * \marble window
        {
            source observable    :  +-1-2-3-4-5-|

            operator "window(2)" : 
                                {   
                                    .+1-2|
                                    .....+3-4|
                                    .........+5-|
                                }
        }
     *
     * \details Actually it is similar to `buffer` but it emits observable instead of container.
     *
     * \param window_size amount of items which every observable would have
     *
     * \return new specific_observable with the window operator as most recent operator.
     * \warning #include <rpp/operators/window.hpp>
     * 
     * \par Example
     * \snippet window.cpp window
	 *
     * \ingroup transforming_operators
     * \see https://reactivex.io/documentation/operators/window.html
     */
    template<typename ...Args>
    auto window(size_t window_size) const & requires is_header_included<window_tag, Args...>
    {
        return window_impl<Type>(*static_cast<const SpecificObservable*>(this), window_size);
    }

    template<typename ...Args>
    auto window(size_t window_size) && requires is_header_included<window_tag, Args...>
    {
        return window_impl<Type>(std::move(*static_cast<SpecificObservable*>(this)), window_size);
    }
};
} // namespace rpp::details
