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

#include <rpp/defs.hpp>
#include <rpp/utils/constraints.hpp>

#include <memory>

namespace rpp::utils
{
template<rpp::constraint::decayed_type First, rpp::constraint::decayed_type Second>
class double_variant_with_monostate
{
public:
    double_variant_with_monostate() = default;
    double_variant_with_monostate(const double_variant_with_monostate& other)
        : m_index{other.m_index}
    {
        switch (m_index) {
            case Index::Monostate:
                std::construct_at(&u.m_monostate, other.u.m_monostate);
                break;
            case Index::FirstValue:
                std::construct_at(&u.m_first, other.u.m_first);
                break;
            case Index::SecondValue:
                std::construct_at(&u.m_second, other.u.m_second);
                break;
        }
    }
    double_variant_with_monostate(double_variant_with_monostate&& other)
        : m_index{other.m_index}
    {
        switch (m_index) {
            case Index::Monostate:
                std::construct_at(&u.m_monostate, other.u.m_monostate);
                break;
            case Index::FirstValue:
                std::construct_at(&u.m_first, other.u.m_first);
                break;
            case Index::SecondValue:
                std::construct_at(&u.m_second, other.u.m_second);
                break;
        }
    }

    ~double_variant_with_monostate()
    {
        destroy();
    }

    const double_variant_with_monostate& operator=(std::monostate)
    {
        destroy();
        m_index = Index::Monostate;
        std::construct_at(&u.m_monostate);
        return *this;
    }

    template<rpp::constraint::decayed_same_as<First> T>
    const double_variant_with_monostate& operator=(T&& v)
    {
        if (m_index == Index::FirstValue)
            u.m_first = std::forward<T>(v);
        else
        {
            destroy();
            std::construct_at(&u.m_first, std::forward<T>(v));
            m_index = Index::FirstValue;
        }
        return *this;
    }

    template<rpp::constraint::decayed_same_as<Second> T>
    const double_variant_with_monostate& operator=(T&& v)
    {
        if (m_index == Index::SecondValue)
            u.m_second = std::forward<T>(v);
        else
        {
            destroy();
            std::construct_at(&u.m_second, std::forward<T>(v));
            m_index = Index::SecondValue;
        }
        return *this;
    }

    template<typename Fn>
        requires (std::invocable<Fn, First&> && std::invocable<Fn, Second&> && std::invocable<Fn, std::monostate&>)
    auto visit(Fn&& fn)
    {
        switch (m_index) {
            case Index::Monostate:
                return std::forward<Fn>(fn)(u.m_monostate);
            case Index::FirstValue:
                return std::forward<Fn>(fn)(u.m_first);
            case Index::SecondValue:
                return std::forward<Fn>(fn)(u.m_second);
        }
    }

    template<typename Fn>
        requires (std::invocable<Fn, const First&> && std::invocable<Fn, const Second&> && std::invocable<Fn, const std::monostate&>)
    auto visit(Fn&& fn) const
    {
        switch (m_index) {
            case Index::Monostate:
                return std::forward<Fn>(fn)(u.m_monostate);
            case Index::FirstValue:
                return std::forward<Fn>(fn)(u.m_first);
            case Index::SecondValue:
                return std::forward<Fn>(fn)(u.m_second);
        }
    }

private:
    void destroy()
    {
        switch (m_index) {
            case Index::Monostate:
                break;
            case Index::FirstValue:
                return std::destroy_at(&u.m_first);
            case Index::SecondValue:
                return std::destroy_at(&u.m_second);
        }
    }

private:
    union U {
        U() {};
        ~U() {}

        std::monostate m_monostate{};
        First m_first;
        Second m_second;
    } u;

    enum class Index : uint8_t {
        Monostate = 0,
        FirstValue = 1,
        SecondValue = 2
    };
    Index m_index : 2{};
};
}