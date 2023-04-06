//                  ReactivePlusPlus library
//
//          Copyright Aleksey Loginov 2022 - present.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)
//
// Project home: https://github.com/victimsnino/ReactivePlusPlus
//

#include <snitch/snitch.hpp>
#include <rpp/sources/create.hpp>
#include <exception>
#include "mock_observer.hpp"
#include "rpp/disposables/composite_disposable.hpp"

TEST_CASE("subscribe as operator")
{
    mock_observer_strategy<int> mock{};
    auto observable = rpp::source::create<int>([](const auto&){});

    static_assert(std::is_same_v<decltype(observable | rpp::operators::subscribe(mock.get_observer())), void>);
    static_assert(std::is_same_v<decltype(observable | rpp::operators::subscribe(rpp::composite_disposable{}, mock.get_observer())), rpp::composite_disposable>);
    static_assert(std::is_same_v<decltype(observable | rpp::operators::subscribe([](const auto&){}, [](const std::exception_ptr&){}, [](){})), void>);
    static_assert(std::is_same_v<decltype(observable | rpp::operators::subscribe(rpp::composite_disposable{}, [](const auto&){}, [](const std::exception_ptr&){}, [](){})), rpp::composite_disposable>);
}
