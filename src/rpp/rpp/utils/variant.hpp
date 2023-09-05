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

#include <rpp/defs.hpp>
#include <rpp/utils/constraints.hpp>
#include <rpp/utils/tuple.hpp>

namespace rpp::utils
{
template<rpp::constraint::decayed_type ...Types>
class variant
{
    using types_as_tuple = rpp::utils::tuple<Types...>;
public:
    variant() 
    {
        std::construct_at(static_cast<typename types_as_tuple::template type_at_index_t<0>*>(m_buff));
    }

    template<typename Fn>
        requires (std::invocable<Fn, Types> && ...)
    auto apply(Fn&& fn)
    {
        return fn()
    }

private:
    alignas(Types...) std::byte m_buff[std::max({sizeof(Types)...})];
    size_t                      m_index{};
};
}