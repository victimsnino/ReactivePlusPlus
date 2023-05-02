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

#include <rpp/observables/fwd.hpp>
#include <rpp/utils/function_traits.hpp>
#include <rpp/utils/constraints.hpp>
#include <rpp/utils/utils.hpp>

namespace rpp::operators
{
auto take(size_t count);

template<typename...Args>
class subscribe;

template<typename Fn>
    requires (!utils::is_not_template_callable<Fn> || !std::same_as<void, std::invoke_result_t<Fn, utils::convertible_to_any>>)
auto map(Fn&& callable);

template<typename Fn>
    requires (!utils::is_not_template_callable<Fn> || std::same_as<bool, std::invoke_result_t<Fn, utils::convertible_to_any>>)
auto filter(Fn&& callable);

template<typename Fn>
    requires (!utils::is_not_template_callable<Fn> || std::same_as<bool, std::invoke_result_t<Fn, utils::convertible_to_any>>)
auto take_while(Fn&& callable);

inline auto concat();

template<constraint::observable ...TObservables>
    requires (sizeof...(TObservables) >= 1)
auto concat_with(TObservables&&... observables);
}