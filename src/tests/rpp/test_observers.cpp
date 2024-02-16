//                   ReactivePlusPlus library
//
//           Copyright Aleksey Loginov 2023 - present.
//  Distributed under the Boost Software License, Version 1.0.
//     (See accompanying file LICENSE_1_0.txt or copy at
//           https://www.boost.org/LICENSE_1_0.txt)
//
//  Project home: https://github.com/victimsnino/ReactivePlusPlus

#include <snitch/snitch.hpp>

#include <rpp/observers.hpp>

#include "rpp/disposables/fwd.hpp"

#include <memory>
#include <vector>

TEST_CASE("lambda observer works properly as base observer")
{
    std::vector<int> on_next_vals{};
    size_t           on_error{};
    size_t           on_completed{};
    auto             observer = rpp::make_lambda_observer<int>([&](int v) { on_next_vals.push_back(v); },
                                                   [&](const std::exception_ptr&) { ++on_error; },
                                                   [&]() { ++on_completed; });

    auto test_observer = [&](const auto& obs) {
        obs.on_next(1);
        CHECK(on_next_vals == std::vector{1});

        int v{2};
        obs.on_next(v);
        CHECK(on_next_vals == std::vector{1, 2});

        SECTION("send on_error first")
        {
            CHECK(!obs.is_disposed());

            obs.on_error(std::exception_ptr{});
            CHECK(on_error == 1u);
            CHECK(obs.is_disposed());

            obs.on_completed();
            CHECK(on_completed == 0u);
            CHECK(obs.is_disposed());
        }

        SECTION("send on_completed first")
        {
            CHECK(!obs.is_disposed());

            obs.on_completed();
            CHECK(on_completed == 1u);
            CHECK(obs.is_disposed());

            obs.on_error(std::exception_ptr{});
            CHECK(on_error == 0u);
            CHECK(obs.is_disposed());
        }
    };

    SECTION("lambda observer obtains callbacks")
    {
        static_assert(!std::is_copy_constructible_v<decltype(observer)>, "lambda observer shouldn't be copy constructible");
        test_observer(observer);
    }

    SECTION("lambda observer obtains callbacks via cast to dynamic_observer")
    {
        auto dynamic_observer = std::move(observer).as_dynamic();
        static_assert(std::is_copy_constructible_v<decltype(dynamic_observer)>, "dynamic observer should be copy constructible");
        test_observer(dynamic_observer);
    }
}

TEST_CASE("as_dynamic keeps disposing")
{
    auto check = [](auto&& observer) {
        SECTION("dispose and convert to dynamic")
        {
            observer.on_completed();
            auto dynamic = std::forward<decltype(observer)>(observer).as_dynamic();
            CHECK(dynamic.is_disposed());
        }

        SECTION("set upstream, convert to dynamic and dispose")
        {
            auto d = rpp::composite_disposable_wrapper::make();
            observer.set_upstream(rpp::disposable_wrapper{d});
            auto dynamic = std::forward<decltype(observer)>(observer).as_dynamic();
            dynamic.on_completed();
            CHECK(d.is_disposed());
        }

        SECTION("convert to dynamic, copy and dispose")
        {
            auto dynamic         = std::forward<decltype(observer)>(observer).as_dynamic();
            auto copy_of_dynamic = dynamic; // NOLINT
            dynamic.on_completed();
            CHECK(copy_of_dynamic.is_disposed());
        }
    };

    SECTION("observer")
    {
        check(rpp::make_lambda_observer<int>([](int) {}, [](const std::exception_ptr&) {}, []() {}));
    }
    SECTION("observer with disposable")
    {
        check(rpp::make_lambda_observer<int>(
            rpp::composite_disposable_wrapper::make(),
            [](int) {},
            [](const std::exception_ptr&) {},
            []() {}));
    }
    SECTION("observer with disposed disposable")
    {
        check(rpp::make_lambda_observer<int>(
            rpp::composite_disposable_wrapper::make(),
            [](int) {},
            [](const std::exception_ptr&) {},
            []() {}));
    }
}

TEST_CASE("observer disposes disposable on termination callbacks")
{
    auto d        = rpp::composite_disposable_wrapper::make();
    auto observer = rpp::make_lambda_observer<int>(
        d,
        [](int) {},
        [](const std::exception_ptr&) {},
        []() {});

    auto upstream = rpp::disposable_wrapper::make<rpp::composite_disposable>();
    observer.set_upstream(upstream);

    CHECK(!d.is_disposed());
    CHECK(!upstream.is_disposed());
    CHECK(!observer.is_disposed());

    SECTION("calling on_error causes disposing of disposables")
    {
        observer.on_error({});
        CHECK(upstream.is_disposed());
        CHECK(d.is_disposed());
        CHECK(observer.is_disposed());
    }

    SECTION("calling on_completed causes disposing of disposables")
    {
        observer.on_completed();
        CHECK(upstream.is_disposed());
        CHECK(d.is_disposed());
        CHECK(observer.is_disposed());
    }
}

TEST_CASE("set_upstream without base disposable makes it main disposalbe")
{
    auto original_observer = rpp::make_lambda_observer<int>([](int) {}, [](const std::exception_ptr&) {}, []() {});

    auto test_observer = [](auto&& observer) {
        auto upstream = rpp::disposable_wrapper::make<rpp::composite_disposable>();
        observer.set_upstream(upstream);
        CHECK(!upstream.is_disposed());
        CHECK(!observer.is_disposed());

        SECTION("calling on_error causes disposing of upstream")
        {
            observer.on_error({});
            CHECK(upstream.is_disposed());
            CHECK(observer.is_disposed());
        }

        SECTION("calling on_completed causes disposing of upstream")
        {
            observer.on_completed();
            CHECK(upstream.is_disposed());
            CHECK(observer.is_disposed());
        }
    };

    SECTION("original observer")
    test_observer(original_observer);

    SECTION("dynamic observer")
    test_observer(std::move(original_observer).as_dynamic());
}

TEST_CASE("set_upstream can be called multiple times")
{
    auto check = [](auto&& observer) {
        auto d1 = rpp::composite_disposable_wrapper::make();
        observer.set_upstream(rpp::disposable_wrapper{d1});
        CHECK(d1.is_disposed() == observer.is_disposed());
        auto d2 = rpp::composite_disposable_wrapper::make();
        observer.set_upstream(rpp::disposable_wrapper{d2});
        CHECK(d1.is_disposed() == observer.is_disposed());
        CHECK(d2.is_disposed() == observer.is_disposed());

        observer.on_completed();
        CHECK(d1.is_disposed());
        CHECK(d2.is_disposed());
    };

    SECTION("observer")
    check(rpp::make_lambda_observer<int>([](int) {}, [](const std::exception_ptr&) {}, []() {}));

    SECTION("observer with disposable")
    check(rpp::make_lambda_observer<int>(
        rpp::composite_disposable_wrapper::make(),
        [](int) {},
        [](const std::exception_ptr&) {},
        []() {}));

    SECTION("observer with empty disposable")
    check(rpp::make_lambda_observer<int>(
        rpp::composite_disposable_wrapper::empty(),
        [](int) {},
        [](const std::exception_ptr&) {},
        []() {}));
}

TEST_CASE("set_upstream depends on base disposable")
{
    auto d                 = rpp::composite_disposable_wrapper::make();
    auto original_observer = rpp::make_lambda_observer<int>(
        d,
        [](int) {},
        [](const std::exception_ptr&) {},
        []() {});

    auto test_observer = [&d](auto&& observer) {
        auto upstream = rpp::disposable_wrapper::make<rpp::composite_disposable>();

        CHECK(!d.is_disposed());
        CHECK(!upstream.is_disposed());
        CHECK(!observer.is_disposed());

        SECTION("disposing of base disposable and setting upstream disposes upstream")
        {
            d.dispose();
            CHECK(!upstream.is_disposed());
            CHECK(observer.is_disposed());
            observer.set_upstream(upstream);
            CHECK(upstream.is_disposed());
            CHECK(observer.is_disposed());
        }

        SECTION("setting upstream and disposing of base disposable disposes upstream")
        {
            observer.set_upstream(upstream);
            CHECK(!upstream.is_disposed());
            CHECK(!observer.is_disposed());
            d.dispose();
            CHECK(upstream.is_disposed());
            CHECK(observer.is_disposed());
        }
    };

    SECTION("original observer")
    test_observer(original_observer);

    SECTION("dynamic observer")
    test_observer(std::move(original_observer).as_dynamic());
}

TEST_CASE("set_upstream disposing when empty base disposable")
{
    auto original_observer = rpp::make_lambda_observer<int>(
        rpp::composite_disposable_wrapper::empty(),
        [](int) {},
        [](const std::exception_ptr&) {},
        []() {});

    auto test_observer = [](auto&& observer) {
        auto upstream = rpp::disposable_wrapper::make<rpp::composite_disposable>();

        CHECK(!upstream.is_disposed());
        CHECK(observer.is_disposed());

        observer.set_upstream(upstream);

        CHECK(upstream.is_disposed());
        CHECK(observer.is_disposed());
    };

    SECTION("original observer")
    test_observer(original_observer);

    SECTION("dynamic observer")
    test_observer(std::move(original_observer).as_dynamic());
}
