//                   ReactivePlusPlus library
// 
//           Copyright Aleksey Loginov 2023 - present.
//  Distributed under the Boost Software License, Version 1.0.
//     (See accompanying file LICENSE_1_0.txt or copy at
//           https://www.boost.org/LICENSE_1_0.txt)
// 
//  Project home: https://github.com/victimsnino/ReactivePlusPlus

#include <snitch/snitch.hpp>
#include <rpp/disposables/composite_disposable.hpp>


TEST_CASE("composite disposable keeps state")
{
    auto d = rpp::composite_disposable{};

    CHECK(!d.is_disposed());


    SECTION("dispose marks disposable as disposed")
    {
        d.dispose();
        CHECK(d.is_disposed());
    }

    SECTION("dispose on copy of disposable marks both disposable as disposed")
    {
        auto copy = d;
        copy.dispose();
        CHECK(copy.is_disposed());
        CHECK(d.is_disposed());
    }

    SECTION("add other disposable")
    {
        auto other = rpp::composite_disposable{};
        CHECK(!other.is_disposed());
        d.add(other);
        SECTION("calling dispose on original disposable forces both of them to be disposed")
        {
            d.dispose();
            CHECK(other.is_disposed());
            CHECK(d.is_disposed());
        }

        SECTION("calling dispose on other disposable forces only other to be disposed")
        {
            other.dispose();
            CHECK(other.is_disposed());
            CHECK(!d.is_disposed());
        }
    }

    SECTION("empty composite_disposable")
    {
        d = rpp::composite_disposable::empty();
        CHECK(d.is_disposed());

        SECTION("adding non disposed disposable to empty forces it to be disposed")
        {
            auto other = rpp::composite_disposable{};
            CHECK(!other.is_disposed());
            d.add(other);
            CHECK(other.is_disposed());
        }
    }
}
