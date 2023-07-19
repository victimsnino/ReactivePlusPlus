//                   ReactivePlusPlus library
//
//           Copyright Aleksey Loginov 2023 - present.
//  Distributed under the Boost Software License, Version 1.0.
//     (See accompanying file LICENSE_1_0.txt or copy at
//           https://www.boost.org/LICENSE_1_0.txt)
//
//  Project home: https://github.com/victimsnino/ReactivePlusPlus

#include <snitch/snitch.hpp>
#include <rpp/disposables/disposable_wrapper.hpp>
#include <rpp/disposables/composite_disposable.hpp>


TEST_CASE("disposable keeps state")
{
    auto d = rpp::disposable_wrapper_impl{std::make_shared<rpp::composite_disposable>()};

    CHECK(!d.is_disposed());


    SECTION("dispose marks disposable as disposed")
    {
        d.dispose();
        CHECK(d.is_disposed());
    }

    SECTION("dispose on copy of disposable marks both disposable as disposed")
    {
        auto copy = d; // NOLINT(performance-unnecessary-copy-initialization)
        copy.dispose();
        CHECK(copy.is_disposed());
        CHECK(d.is_disposed());
    }

    SECTION("add other disposable")
    {
        auto other = std::make_shared<rpp::composite_disposable>();
        CHECK(!other->is_disposed());
        d.add(other);
        SECTION("calling dispose on original disposable forces both of them to be disposed")
        {
            d.dispose();
            CHECK(other->is_disposed());
            CHECK(d.is_disposed());
        }

        SECTION("calling dispose on other disposable forces only other to be disposed")
        {
            other->dispose();
            CHECK(other->is_disposed());
            CHECK(!d.is_disposed());
        }
    }

    SECTION("add disposed disposable")
    {
        auto other = std::make_shared<rpp::composite_disposable>();
        other->dispose();
        d.add(other);
        CHECK(other->is_disposed());
        CHECK(!d.is_disposed());
    }

    SECTION("disposed disposable")
    {
        d.dispose();

        SECTION("adding non disposed disposable to empty forces it to be disposed")
        {
            auto other = std::make_shared<rpp::composite_disposable>();
            CHECK(!other->is_disposed());
            d.add(other);
            CHECK(other->is_disposed());
        }
    }

    SECTION("empty disposable")
    {
        d = rpp::disposable_wrapper_impl<rpp::composite_disposable>{};
        CHECK(d.is_disposed());
        d.dispose();

        SECTION("adding non disposed disposable to empty forces it to be disposed")
        {
            auto other = std::make_shared<rpp::composite_disposable>();
            CHECK(!other->is_disposed());
            d.add(other);
            CHECK(other->is_disposed());
        }
    }

    SECTION("add self") {
        d.add(d.get_original());
        CHECK(!d.is_disposed());
        SECTION("dispose self") {
            d.dispose();
            CHECK(d.is_disposed());
        }
    }

    SECTION("call dispose twice") {
        d.dispose();
        CHECK(d.is_disposed());

        d.dispose();
        CHECK(d.is_disposed());
    }
}
