//                   ReactivePlusPlus library
//
//           Copyright Aleksey Loginov 2023 - present.
//  Distributed under the Boost Software License, Version 1.0.
//     (See accompanying file LICENSE_1_0.txt or copy at
//           https://www.boost.org/LICENSE_1_0.txt)
//
//  Project home: https://github.com/victimsnino/ReactivePlusPlus

#pragma once

#include <rpp/disposables/disposable_wrapper.hpp>
#include <rpp/utils/exceptions.hpp>

#include <array>
#include <vector>

namespace rpp::details::disposables
{
template<size_t Count>
class dynamic_disposables_container
{
public:
    dynamic_disposables_container()
    {
        m_data.reserve(Count);
    }

    void push_back(rpp::disposable_wrapper d)
    {
        m_data.push_back(std::move(d));
    }

    void dispose() const
    {
        for (auto& d : m_data) {
            d.dispose();
            d = rpp::disposable_wrapper{};
        }
    }

private:
    mutable std::vector<rpp::disposable_wrapper> m_data{};
};

template<size_t Count>
class static_disposables_container
{
public:
    static_disposables_container() = default;

    void push_back(const rpp::disposable_wrapper& d)
    {
        if (m_size >= Count)
            throw rpp::utils::more_disposables_than_expected{"static_disposables_container obtained more disposables than expected"};
        m_data[m_size++] = d;
    }

    void dispose() const
    {
        for (size_t i =0; i < m_size; ++i) {
            m_data[i].dispose();
            m_data[i] = rpp::disposable_wrapper{};
        }
    }

private:
    mutable std::array<rpp::disposable_wrapper, Count> m_data{};
    size_t                                             m_size{};
};

struct none_disposables_container
{
    static void push_back(const rpp::disposable_wrapper&)
    {
        throw rpp::utils::more_disposables_than_expected{"none_disposables_container expected none disposables but obtained one"};
    }

    static void dispose() {};
};
}