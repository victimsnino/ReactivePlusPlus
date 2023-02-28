//                  ReactivePlusPlus library
//
//          Copyright Aleksey Loginov 2022 - present.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)
//
// Project home: https://github.com/victimsnino/ReactivePlusPlus
//


#include <catch2/catch_test_macros.hpp>
#include <rpp/disposables.hpp>

SCENARIO("composite disposable keeps state")
{
    GIVEN("composite_disposable")
    {
        auto d = rpp::composite_disposable{};

        CHECK(!d.is_disposed());

        WHEN("call dispose")
        {
            d.dispose();
            THEN("disposable becomes disposed")
            {
                CHECK(d.is_disposed());
            }
        }
        WHEN("copy disposable")
        {
            auto copy = d;
            AND_WHEN("call dispose on copy")
            {
                copy.dispose();
                THEN("both disposables becomes disposed")
                {
                    CHECK(copy.is_disposed());
                    CHECK(d.is_disposed());
                }
            }
        }
        WHEN("add other disposable")
        {
            auto other = rpp::composite_disposable{};
            CHECK(!other.is_disposed());
            d.add(other);
            AND_WHEN("call dispose on original disposable")
            {
                d.dispose();
                THEN("both of them becomes disposed")
                {
                    CHECK(other.is_disposed());
                    CHECK(d.is_disposed());
                }
            }

            AND_WHEN("call dispose on other disposable")
            {
                other.dispose();
                THEN("only other disposable becomes disposed")
                {
                    CHECK(other.is_disposed());
                    CHECK(!d.is_disposed());
                }
            }
        }
    }
    GIVEN("empty composite_disposable")
    {
        auto d = rpp::composite_disposable::empty();
        CHECK(d.is_disposed());

        WHEN("add non disposed disposable to empty")
        {
            auto other = rpp::composite_disposable{};
            CHECK(!other.is_disposed());
            d.add(other);
            THEN("it becomes disposed")
            {
                CHECK(other.is_disposed());
            }
        }
    }
}


