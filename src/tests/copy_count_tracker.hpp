//                  ReactivePlusPlus library
//
//          Copyright Aleksey Loginov 2022 - present.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)
//
// Project home: https://github.com/victimsnino/ReactivePlusPlus
//

#pragma once

#include "rpp/sources/create.hpp"

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

    auto get_observable(size_t count = 1)
    {
        return rpp::source::create<copy_count_tracker>([this, count](const auto& sub)
        {
            for (size_t i = 0; i < count; ++i)
                sub.on_next(*this);
            sub.on_completed();
        });
    }

    auto get_observable_for_move(size_t count = 1)
    {
        return rpp::source::create<copy_count_tracker>([this, count](const auto& sub)
        {
            for (size_t i = 0; i < count; ++i)
                sub.on_next(std::move(*this));
            sub.on_completed();
        });
    }

    int get_copy_count() const { return _state->copy_count; }
    int get_move_count() const { return _state->move_count; }

private:
    struct state
    {
        int copy_count = 0;
        int move_count = 0;
    };

    std::shared_ptr<state> _state;
};
