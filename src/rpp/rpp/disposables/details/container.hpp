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
#include <algorithm>
#include <vector>

namespace rpp::details::disposables
{
class dynamic_disposables_container_base
{
public:
    explicit dynamic_disposables_container_base(size_t count)
    {
        m_data.reserve(count);
    }

    void push_back(const rpp::disposable_wrapper& d)
    {
        m_data.push_back(d);
    }

    void push_back(rpp::disposable_wrapper&& d)
    {
        m_data.push_back(std::move(d));
    }

    void remove(const rpp::disposable_wrapper& d)
    {
        m_data.erase(std::remove(m_data.begin(), m_data.end(), d), m_data.end());
    }

    void dispose() const
    {
        for (auto& d : m_data) {
            d.dispose();
        }
    }

    void clear()
    {
        m_data.clear();
    }

private:
    mutable std::vector<rpp::disposable_wrapper> m_data{};
};

template<size_t Count>
class dynamic_disposables_container : public dynamic_disposables_container_base
{
public:
    dynamic_disposables_container() 
        : dynamic_disposables_container_base{Count}
    {}
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

    void push_back(rpp::disposable_wrapper&& d)
    {
        if (m_size >= Count)
            throw rpp::utils::more_disposables_than_expected{"static_disposables_container obtained more disposables than expected"};
        m_data[m_size++] = std::move(d);
    }

    void remove(const rpp::disposable_wrapper& d)
    {
        auto itr = std::remove(m_data.begin(), m_data.end(), d);
        while(itr != m_data.end()) {
            (*itr++) = disposable_wrapper{};
            --m_size;
        }
    }

    void dispose() const
    {
        for (size_t i =0; i < m_size; ++i) {
            m_data[i].dispose();
        }
    }

    void clear()
    {
        m_data = std::array<rpp::disposable_wrapper, Count>{};
        m_size = 0;
    }

private:
    mutable std::array<rpp::disposable_wrapper, Count> m_data{};
    size_t                                             m_size{};
};

struct none_disposables_container
{
    [[noreturn]] static void push_back(const rpp::disposable_wrapper&)
    {
        throw rpp::utils::more_disposables_than_expected{"none_disposables_container expected none disposables but obtained one"};
    }

    static void remove(const rpp::disposable_wrapper&) {}
    static void dispose() {}
    static void clear() {}
};
}