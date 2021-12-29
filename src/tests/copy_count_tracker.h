// MIT License
// 
// Copyright (c) 2021 Aleksey Loginov
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#pragma once

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
