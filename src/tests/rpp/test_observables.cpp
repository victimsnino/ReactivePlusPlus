//                   ReactivePlusPlus library
//
//           Copyright Aleksey Loginov 2023 - present.
//  Distributed under the Boost Software License, Version 1.0.
//     (See accompanying file LICENSE_1_0.txt or copy at
//           https://www.boost.org/LICENSE_1_0.txt)
//
//  Project home: https://github.com/victimsnino/ReactivePlusPlus

#include "rpp/disposables/composite_disposable.hpp"
#include <snitch/snitch.hpp>
#include <rpp/observables.hpp>

TEST_CASE("lambda observable works properly as base_observable")
{
    size_t on_subscribe_called{};
    auto observable = rpp::make_lambda_observable<int>([&](auto&& observer)
    {
        ++on_subscribe_called;
        observer.on_next(1);
        observer.on_completed();
    });

    SECTION("subscribe valid observer") 
    {
        std::vector<int> on_next_vals{};
        size_t           on_error{};
        size_t           on_completed{};

        observable.subscribe([&](int v) { on_next_vals.push_back(v); },
                             [&](const std::exception_ptr&) { ++on_error; },
                             [&]() { ++on_completed; });

        CHECK(on_subscribe_called == 1u);
        CHECK(on_next_vals == std::vector{1});
        CHECK(on_error == 0u);
        CHECK(on_completed == 1u);
    }

    SECTION("subscribe disposed observer")
    {
        observable.subscribe(rpp::composite_disposable::empty(), [](int) {}, [](const std::exception_ptr&) {}, []() {});

        CHECK(on_subscribe_called == 0u);
    }
}