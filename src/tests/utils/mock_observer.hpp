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

#include "rpp/disposables/fwd.hpp"
#include <rpp/observers/dynamic_observer.hpp>

#include <vector>

template<typename Type>
class mock_observer final : public rpp::interface_observer<Type>
{
public:
    explicit mock_observer(bool copy_values = true) : m_state{std::make_shared<State>(copy_values)} {}

    void on_next(const Type& v) const noexcept override
    {
        ++m_state->m_on_next_const_ref_count;
        if (m_state->m_copy_values)
            m_state->vals.push_back(v);
    }

    void on_next(Type&& v) const noexcept override
    {
        ++m_state->m_on_next_move_count;
        if (m_state->m_copy_values)
            m_state->vals.push_back(std::move(v));
    }

    void on_error(const std::exception_ptr&) const noexcept override { ++m_state->m_on_error_count; }
    void on_completed() const noexcept override { ++m_state->m_on_completed_count; }

    bool is_disposed() const noexcept override { return false; }
    void set_resource(const rpp::composite_disposable&) noexcept override {}

    rpp::dynamic_observer<Type> as_dynamic() && noexcept override { return {std::make_shared<mock_observer>(std::move(*this))}; }

    [[nodiscard]] size_t get_total_on_next_count() const { return m_state->m_on_next_const_ref_count + m_state->m_on_next_move_count; }
    [[nodiscard]] size_t get_on_next_const_ref_count() const { return m_state->m_on_next_const_ref_count; }
    [[nodiscard]] size_t get_on_next_move_count() const { return m_state->m_on_next_move_count; }
    [[nodiscard]] size_t get_on_error_count() const { return m_state->m_on_error_count; }
    [[nodiscard]] size_t get_on_completed_count() const { return m_state->m_on_completed_count; }

    std::vector<Type> get_received_values() const {return m_state->vals; }

private:
    struct State
    {
        explicit State(bool copy_values)
            : m_copy_values{copy_values} {}

        bool   m_copy_values             = true;
        size_t m_on_next_const_ref_count = 0;
        size_t m_on_next_move_count      = 0;
        size_t m_on_error_count          = 0;
        size_t m_on_completed_count      = 0;

        std::vector<Type> vals{};
    };

    std::shared_ptr<State> m_state{};
};
