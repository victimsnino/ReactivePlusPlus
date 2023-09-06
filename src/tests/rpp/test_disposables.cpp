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
#include <rpp/disposables/refcount_disposable.hpp>
#include <rpp/disposables/composite_disposable.hpp>

namespace {
struct custom_disposable : public rpp::interface_disposable
{
    custom_disposable() = default;

    bool is_disposed() const noexcept final { return dispose_count > 1; }

    void dispose() noexcept final { ++dispose_count; }

    size_t dispose_count{};
};
}

TEST_CASE("disposable keeps state")
{
    auto d = rpp::composite_disposable_wrapper{std::make_shared<rpp::composite_disposable>()};

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
        d = rpp::composite_disposable_wrapper{};
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

TEST_CASE("refcount disposable dispose underlying in case of reaching zero")
{
    auto underlying = std::make_shared<custom_disposable>();
    auto refcount = std::make_shared<rpp::refcount_disposable>(underlying);

    CHECK(!underlying->is_disposed());
    CHECK(!refcount->is_disposed());

    SECTION("disposing refcount as is disposes underlying")
    {
        refcount->dispose();

        CHECK(underlying->dispose_count == 1);
        CHECK(refcount->is_disposed());

        SECTION("additional disposing does nothing")
        {
            refcount->dispose();
            CHECK(underlying->dispose_count == 1);
            CHECK(refcount->is_disposed());
        }
        SECTION("addref and disposing does nothing")
        {
            refcount->add_ref();
            refcount->dispose();
            CHECK(underlying->dispose_count == 1);
            CHECK(refcount->is_disposed());
        }
    }

    SECTION("disposing underlying not disposes refcount")
    {
        underlying->dispose();

        CHECK(underlying->dispose_count == 1);
        CHECK(!refcount->is_disposed());
    }

    SECTION("add_ref prevents immediate disposing")
    {
        size_t count = 5;
        for (size_t i = 0; i < count; ++i)
            refcount->add_ref();

        for (size_t i = 0; i < count + 1; ++i)
        {
            CHECK(!underlying->is_disposed());
            CHECK(!refcount->is_disposed());

            refcount->dispose();
        }

        CHECK(underlying->dispose_count == 1);
        CHECK(refcount->is_disposed());
    }
}