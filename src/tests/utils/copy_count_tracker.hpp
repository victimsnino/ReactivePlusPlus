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

#include <rpp/sources/create.hpp>
#include <snitch/snitch.hpp>

#include <memory>

class copy_count_tracker
{
public:
    copy_count_tracker()
        : _state(std::make_shared<state>()) {}

    copy_count_tracker(const copy_count_tracker& other)
        : _state{other._state}
    {
        ++_state->copy_count;
    }

    copy_count_tracker(copy_count_tracker&& other) noexcept
        : _state{other._state} // NOLINT(performance-move-constructor-init)
    {
        ++_state->move_count;
    }

    copy_count_tracker& operator=(const copy_count_tracker& other)
    {
        if (this == &other)
            return *this;
        _state = other._state;
        ++_state->copy_count;
        return *this;
    }

    copy_count_tracker& operator=(copy_count_tracker&& other) noexcept
    {
        if (this == &other)
            return *this;
        _state = other._state;
        ++_state->move_count;
        return *this;
    }

    bool operator==(const copy_count_tracker& other) const
    {
        return _state == other._state;
    }

    bool operator!=(const copy_count_tracker& other) const { return !(*this == other); }

    auto get_observable(size_t count = 1)
    {
        return rpp::source::create<copy_count_tracker>([this, count](const auto& sub)
        {
            for (size_t i = 0; i < count && !sub.is_disposed(); ++i)
                sub.on_next(*this);
            sub.on_completed();
        });
    }

    auto get_observable_for_move(size_t count = 1)
    {
        return rpp::source::create<copy_count_tracker>([this, count](const auto& sub)
        {
            for (size_t i = 0; i < count && !sub.is_disposed(); ++i)
                sub.on_next(std::move(*this));
            sub.on_completed();
        });
    }

    struct state
    {
        int copy_count = 0;
        int move_count = 0;
    };

    struct test_expectations
    {
        state send_by_copy{};
        state send_by_move{};
    };


    static void test_operator(auto&& op, test_expectations expectations, size_t count = 1)
    {
        copy_count_tracker tracker{};
        SECTION("send value by copy")
        {
            tracker.get_observable(count) | op | rpp::ops::subscribe([](auto){});
            CHECK(tracker.get_copy_count() == expectations.send_by_copy.copy_count);
            CHECK(tracker.get_move_count() == expectations.send_by_copy.move_count);
        }

        SECTION("send value by move")
        {
            tracker.get_observable_for_move(count) | op | rpp::ops::subscribe([](auto){});
            CHECK(tracker.get_copy_count() == expectations.send_by_move.copy_count);
            CHECK(tracker.get_move_count() == expectations.send_by_move.move_count);
        }
    }

    int get_copy_count() const { return _state->copy_count; }
    int get_move_count() const { return _state->move_count; }

private:
    std::shared_ptr<state> _state;
};
