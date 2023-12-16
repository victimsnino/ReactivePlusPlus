//                  ReactivePlusPlus library
//
//          Copyright Aleksey Loginov 2023 - present.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)
//
// Project home: https://github.com/victimsnino/ReactivePlusPlus
//

#include <snitch/snitch.hpp>
#include <rpp/sources/just.hpp>
#include <rpp/sources/concat.hpp>
#include <rpp/sources/create.hpp>
#include <rpp/sources/just.hpp>
#include <rpp/schedulers/immediate.hpp>
#include <rpp/subjects/publish_subject.hpp>

#include <rpp/operators/concat.hpp>
#include <snitch/snitch_macros_check.hpp>
#include <snitch/snitch_macros_test_case.hpp>


#include <rpp/disposables/composite_disposable.hpp>
#include <rpp/disposables/disposable_wrapper.hpp>

#include "mock_observer.hpp"
#include "copy_count_tracker.hpp"
#include "disposable_observable.hpp"
#include "snitch_logging.hpp"

#include <memory>
#include <optional>
#include <stdexcept>

struct my_container_with_error : std::vector<rpp::dynamic_observable<int>>
{
    using std::vector<rpp::dynamic_observable<int>>::vector;
    std::vector<rpp::dynamic_observable<int>>::const_iterator begin() const { throw std::runtime_error{""}; }
};

struct my_container_with_error_on_increment
{
public:
    class iterator
    {
    public:
        iterator(const my_container_with_error_on_increment* container) : m_container{container} {}

        using iterator_category = std::input_iterator_tag;
        using difference_type   = std::ptrdiff_t;
        using value_type        = rpp::dynamic_observable<int>;
        using pointer           = rpp::dynamic_observable<int>*;

        const value_type& operator*() const { return m_container->m_obs; }
        iterator& operator++() { throw std::runtime_error{""}; }
        iterator operator++(int) { throw std::runtime_error{""}; }

        bool operator==(const iterator&) const {return false;};
        bool operator!=(const iterator&) const {return true;};

    private:
        const my_container_with_error_on_increment* m_container;
    };

    iterator begin() const { return {this}; }
    iterator end() const { return {nullptr}; }

private:
    rpp::dynamic_observable<int> m_obs = rpp::source::just(1).as_dynamic();
};

TEMPLATE_TEST_CASE("concat as source", "", rpp::memory_model::use_stack, rpp::memory_model::use_shared)
{
    mock_observer_strategy<int> mock{};
    SECTION("concat of solo observable")
    {
        auto observable = rpp::source::concat<TestType>(rpp::source::just<TestType>(1, 2));
        observable.subscribe(mock);

        CHECK(mock.get_received_values() == std::vector{1,2});
        CHECK(mock.get_on_error_count() == 0);
        CHECK(mock.get_on_completed_count() == 1);
    }
    SECTION("concat of multiple same observables")
    {
        auto observable = rpp::source::concat<TestType>(rpp::source::just<TestType>(1, 2), rpp::source::just<TestType>(1, 2));
        observable.subscribe(mock);

        CHECK(mock.get_received_values() == std::vector{1,2, 1, 2});
        CHECK(mock.get_on_error_count() == 0);
        CHECK(mock.get_on_completed_count() == 1);
    }
    SECTION("concat of multiple different observables")
    {
        auto observable = rpp::source::concat<TestType>(rpp::source::just<TestType>(1, 2), rpp::source::just<TestType>(1));
        observable.subscribe(mock);

        CHECK(mock.get_received_values() == std::vector{1,2,1});
        CHECK(mock.get_on_error_count() == 0);
        CHECK(mock.get_on_completed_count() == 1);
    }
    SECTION("concat of array of different observables")
    {
        auto observable = rpp::source::concat<TestType>(std::array{rpp::source::just<TestType>(1, 2), rpp::source::just<TestType>(1, 1)});
        observable.subscribe(mock);

        CHECK(mock.get_received_values() == std::vector{1,2,1, 1});
        CHECK(mock.get_on_error_count() == 0);
        CHECK(mock.get_on_completed_count() == 1);
    }
    SECTION("concat stop if no completion")
    {
        std::optional<rpp::dynamic_observer<int>> observer{};
        auto observable = rpp::source::concat<TestType>(rpp::source::just<TestType>(1, 2), rpp::source::create<int>([&](auto&& obs){ observer.emplace(std::forward<decltype(obs)>(obs).as_dynamic()); }), rpp::source::just<TestType>(3));
        observable.subscribe(mock);

        CHECK(mock.get_received_values() == std::vector{1,2});
        CHECK(mock.get_on_error_count() == 0);
        CHECK(mock.get_on_completed_count() == 0);
        REQUIRE(observer.has_value());

        SECTION("send completion later")
        {
            observer->on_completed();

            CHECK(mock.get_received_values() == std::vector{1,2,3});
            CHECK(mock.get_on_error_count() == 0);
            CHECK(mock.get_on_completed_count() == 1);
        }
        SECTION("send emission later")
        {
            observer->on_next(10);

            CHECK(mock.get_received_values() == std::vector{1,2,10});
            CHECK(mock.get_on_error_count() == 0);
            CHECK(mock.get_on_completed_count() == 0);
        }
        SECTION("send error later")
        {
            observer->on_error({});

            CHECK(mock.get_received_values() == std::vector{1,2});
            CHECK(mock.get_on_error_count() == 1);
            CHECK(mock.get_on_completed_count() == 0);
        }
    }
    SECTION("concat stoped if disposed")
    {
        auto d = std::make_shared<rpp::composite_disposable>();
        auto observable =
            rpp::source::concat<TestType>(rpp::source::just<TestType>(1),
                                          rpp::source::create<int>([&](auto&& obs) { d->dispose(); obs.on_completed(); }),
                                          rpp::source::create<int>([&](auto&&) { FAIL("Shouldn't be called"); }),
                                          rpp::source::just<TestType>(3));
        observable.subscribe(rpp::composite_disposable_wrapper{d}, mock);

        CHECK(mock.get_received_values() == std::vector{1});
        CHECK(mock.get_on_error_count() == 0);
        CHECK(mock.get_on_completed_count() == 0);
    }

    SECTION("concat tracks actual upstream")
    {
        auto d = std::make_shared<rpp::composite_disposable>();
        auto d1 = std::make_shared<rpp::composite_disposable>();

        auto observable = rpp::source::concat<TestType>(rpp::source::create<int>([&](auto&& obs)
        {
            obs.set_upstream(rpp::disposable_wrapper{d1});

            CHECK(!d->is_disposed());
            CHECK(!d1->is_disposed());

            d->dispose();

            CHECK(d->is_disposed());
            CHECK(d1->is_disposed());

        }));
        observable.subscribe(rpp::composite_disposable_wrapper{d}, mock);
    }

    SECTION("concat tracks actual upstream for 2 upstreams")
    {
        auto d = std::make_shared<rpp::composite_disposable>();
        auto d1 = std::make_shared<rpp::composite_disposable>();
        auto d2 = std::make_shared<rpp::composite_disposable>();

        auto observable =
            rpp::source::concat<TestType>(rpp::source::create<int>([&](auto&& obs) { obs.set_upstream(rpp::disposable_wrapper{d1}); obs.on_completed(); }),
                                          rpp::source::create<int>([&](auto&& obs)
                                          {
                                            obs.set_upstream(rpp::disposable_wrapper{d2});

                                            CHECK(!d->is_disposed());
                                            CHECK(d1->is_disposed());
                                            CHECK(!d2->is_disposed());

                                            d->dispose();

                                            CHECK(d->is_disposed());
                                            CHECK(d2->is_disposed());
                                          }));

        observable.subscribe(rpp::composite_disposable_wrapper{d}, mock);
    }

    SECTION("container with error on begin")
    {
        rpp::source::concat<TestType>(my_container_with_error{}).subscribe(mock);

        CHECK(mock.get_on_error_count() == 1);
    }

    SECTION("container with error on increment")
    {
        rpp::source::concat<TestType>(my_container_with_error_on_increment{}).subscribe(mock);

        CHECK(mock.get_on_error_count() == 1);
    }
}

TEST_CASE("concat as operator")
{
    mock_observer_strategy<int> mock{};
    SECTION("concat of solo observable")
    {
        auto observable = rpp::source::just(rpp::source::just(1, 2)) | rpp::operators::concat();
        observable.subscribe(mock);

        CHECK(mock.get_received_values() == std::vector{1,2});
        CHECK(mock.get_on_error_count() == 0);
        CHECK(mock.get_on_completed_count() == 1);
    }
    SECTION("concat of multiple same observables")
    {
        auto observable = rpp::source::just(rpp::source::just(1, 2), rpp::source::just(1, 2)) | rpp::operators::concat();
        observable.subscribe(mock);

        CHECK(mock.get_received_values() == std::vector{1,2, 1, 2});
        CHECK(mock.get_on_error_count() == 0);
        CHECK(mock.get_on_completed_count() == 1);
    }
    SECTION("concat of multiple different observables")
    {
        auto observable = rpp::source::just(rpp::source::just(1, 2).as_dynamic(), rpp::source::just(1).as_dynamic()) | rpp::operators::concat();
        observable.subscribe(mock);

        CHECK(mock.get_received_values() == std::vector{1,2,1});
        CHECK(mock.get_on_error_count() == 0);
        CHECK(mock.get_on_completed_count() == 1);
    }
    SECTION("concat of array of different observables")
    {
        auto observable = rpp::source::from_iterable(std::array{rpp::source::just(1, 2), rpp::source::just(1, 1)}) | rpp::operators::concat();
        observable.subscribe(mock);

        CHECK(mock.get_received_values() == std::vector{1,2,1, 1});
        CHECK(mock.get_on_error_count() == 0);
        CHECK(mock.get_on_completed_count() == 1);
    }
    SECTION("concat stop if no completion")
    {
        std::optional<rpp::dynamic_observer<int>> observer{};
        auto                                      observable = rpp::source::just(rpp::source::just(1, 2).as_dynamic(), rpp::source::create<int>([&](auto&& obs) { observer.emplace(std::forward<decltype(obs)>(obs).as_dynamic()); }).as_dynamic(), rpp::source::just(3).as_dynamic()) | rpp::operators::concat();
        observable.subscribe(mock);

        CHECK(mock.get_received_values() == std::vector{1,2});
        CHECK(mock.get_on_error_count() == 0);
        CHECK(mock.get_on_completed_count() == 0);
        REQUIRE(observer.has_value());

        SECTION("send completion later")
        {
            observer->on_completed();

            CHECK(mock.get_received_values() == std::vector{1,2,3});
            CHECK(mock.get_on_error_count() == 0);
            CHECK(mock.get_on_completed_count() == 1);
        }
        SECTION("send emission later")
        {
            observer->on_next(10);

            CHECK(mock.get_received_values() == std::vector{1,2,10});
            CHECK(mock.get_on_error_count() == 0);
            CHECK(mock.get_on_completed_count() == 0);
        }
        SECTION("send error later")
        {
            observer->on_error({});

            CHECK(mock.get_received_values() == std::vector{1,2});
            CHECK(mock.get_on_error_count() == 1);
            CHECK(mock.get_on_completed_count() == 0);
        }
    }
    SECTION("concat stoped if disposed")
    {
        auto d = std::make_shared<rpp::composite_disposable>();
        auto observable =
            rpp::source::just(rpp::source::just(1).as_dynamic(),
                              rpp::source::create<int>([&](auto&& obs) { d->dispose(); obs.on_completed(); }).as_dynamic(),
                              rpp::source::create<int>([&](auto&&) { FAIL("Shouldn't be called"); }).as_dynamic(),
                              rpp::source::just(3).as_dynamic())
            | rpp::operators::concat();
        observable.subscribe(rpp::composite_disposable_wrapper{d}, mock);

        CHECK(mock.get_received_values() == std::vector{1});
        CHECK(mock.get_on_error_count() == 0);
        CHECK(mock.get_on_completed_count() == 0);
    }
    SECTION("concat tracks actual upstream")
    {
        auto d = std::make_shared<rpp::composite_disposable>();
        auto d1 = std::make_shared<rpp::composite_disposable>();

        auto observable = rpp::source::just(rpp::source::create<int>([&](auto&& obs)
        {
            obs.set_upstream(rpp::disposable_wrapper{d1});

            CHECK(!d->is_disposed());
            CHECK(!d1->is_disposed());

            d->dispose();

            CHECK(d->is_disposed());
            CHECK(d1->is_disposed());

        })) | rpp::operators::concat();
        observable.subscribe(rpp::composite_disposable_wrapper{d}, mock);
    }
    SECTION("concat tracks actual upstream for 2 upstreams")
    {
        auto d = std::make_shared<rpp::composite_disposable>();
        auto d1 = std::make_shared<rpp::composite_disposable>();
        auto d2 = std::make_shared<rpp::composite_disposable>();

        auto observable =
            rpp::source::just(rpp::source::create<int>([&](auto&& obs) { obs.set_upstream(rpp::disposable_wrapper{d1}); obs.on_completed(); }).as_dynamic(),
                              rpp::source::create<int>([&](auto&& obs) {
                                  obs.set_upstream(rpp::disposable_wrapper{d2});

                                  CHECK(!d->is_disposed());
                                  CHECK(d1->is_disposed());
                                  CHECK(!d2->is_disposed());

                                  d->dispose();

                                  CHECK(d->is_disposed());
                                  CHECK(d2->is_disposed());
                              }).as_dynamic())
            | rpp::operators::concat();

        observable.subscribe(rpp::composite_disposable_wrapper{d}, mock);
    }
}

TEST_CASE("concat as operator async completiton")
{
    mock_observer_strategy<int> mock{};
    rpp::subjects::publish_subject<int> subj{};
    rpp::source::just(subj.get_observable().as_dynamic(), rpp::source::just(100).as_dynamic()) 
        | rpp::ops::concat()
        | rpp::ops::subscribe(mock);

    SECTION("nothing happens before emitting values from subj")
    {
        CHECK(mock.get_received_values() == std::vector<int>{});
        CHECK(mock.get_on_error_count() == 0);
        CHECK(mock.get_on_completed_count() == 0);
    }
    subj.get_observer().on_next(1);
    SECTION("observer see first value from subject")
    {
        CHECK(mock.get_received_values() == std::vector<int>{1});
        CHECK(mock.get_on_error_count() == 0);
        CHECK(mock.get_on_completed_count() == 0);
    }
    subj.get_observer().on_completed();
    SECTION("observer see values from first observable when first completes")
    {
        CHECK(mock.get_received_values() == std::vector<int>{1, 100});
        CHECK(mock.get_on_error_count() == 0);
        CHECK(mock.get_on_completed_count() == 1);
    }
}

TEST_CASE("concat doesn't produce extra copies")
{
    copy_count_tracker tracker{};
    auto source = rpp::source::just(rpp::schedulers::immediate{}, tracker);
    auto               initial_copy = tracker.get_copy_count();
    auto               initial_move = tracker.get_move_count();

    SECTION("pass source via copy")
    {
        rpp::source::concat(source) | rpp::ops::subscribe([](const copy_count_tracker&){});
        CHECK(tracker.get_copy_count() - initial_copy == 2); // 1 copy to observable + 1 copy to observer
        CHECK(tracker.get_move_count() - initial_move == 0);
    }
}

TEST_CASE("concat of iterable doesn't produce extra copies")
{
    copy_count_tracker tracker{};
    auto               source       = std::array{rpp::source::just(rpp::schedulers::immediate{}, tracker)};
    auto               initial_copy = tracker.get_copy_count();
    auto               initial_move = tracker.get_move_count();

    SECTION("pass source via copy")
    {
        rpp::source::concat(source) | rpp::ops::subscribe([](const copy_count_tracker&){});
        CHECK(tracker.get_copy_count() - initial_copy == 2); // 1 copy to observable + 1 copy to observer
        CHECK(tracker.get_move_count() - initial_move == 0);
    }
}

TEST_CASE("concat satisfies disposable contracts")
{
    test_operator_over_observable_with_disposable<int>([](auto&& observable){return rpp::source::concat(observable);});
}