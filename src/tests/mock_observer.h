// MIT License
// 
// Copyright (c) 2022 Aleksey Loginov
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

#include <rpp/observers/dynamic_observer.h>

#include <rpp/observers/interface_observer.h>

#include <vector>

template<typename Type>
class mock_observer : public rpp::interface_observer<Type>
{
public:
    mock_observer(bool copy_values = true) : m_state{std::make_shared<State>(copy_values)} {}

    void on_next(const Type& v) const override
    {
        ++m_state->m_on_next_const_ref_count;
        if (m_state->m_copy_values)
            m_state->vals.push_back(v);
    }

    void on_next(Type&& v) const override
    {
        ++m_state->m_on_next_move_count;
        if (m_state->m_copy_values)
            m_state->vals.push_back(std::move(v));
    }

    void on_error(const std::exception_ptr&) const override { ++m_state->m_on_error_count; }
    void on_completed() const override { ++m_state->m_on_completed_count; }

    size_t get_total_on_next_count() const { return m_state->m_on_next_const_ref_count + m_state->m_on_next_move_count; }
    size_t get_on_next_const_ref_count() const { return m_state->m_on_next_const_ref_count; }
    size_t get_on_next_move_count() const { return m_state->m_on_next_move_count; }
    size_t get_on_error_count() const { return m_state->m_on_error_count; }
    size_t get_on_completed_count() const { return m_state->m_on_completed_count; }

    std::vector<Type> get_received_values() const {return m_state->vals; }

    auto as_dynamic() const {return rpp::dynamic_observer<Type>{*this};}

private:
    struct State
    {
        bool   m_copy_values             = true;
        size_t m_on_next_const_ref_count = 0;
        size_t m_on_next_move_count      = 0;
        size_t m_on_error_count          = 0;
        size_t m_on_completed_count      = 0;

        std::vector<Type> vals{};
    };

    std::shared_ptr<State> m_state{};
};
