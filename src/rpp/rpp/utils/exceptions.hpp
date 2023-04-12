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

#include <stdexcept>

namespace rpp::utils
{
struct error_set_upstream_calle_twice : std::logic_error
{
    error_set_upstream_calle_twice() : std::logic_error{"set_upstream called twice, but expected only once"} {}
};
}