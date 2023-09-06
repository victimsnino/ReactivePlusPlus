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

#include <rpp/disposables/disposable_wrapper.hpp>
#include <rpp/utils/utils.hpp>
#include <rpp/utils/functors.hpp>

#include <variant>
#include <vector>

namespace rpp::details
{
class disposables_variant
{
public:
    disposables_variant() = default;

    void add(rpp::disposable_wrapper d)
    {
        visit(rpp::utils::overloaded{[&](rpp::disposable_wrapper& current)
                                     {
                                         if (current.is_disposed())
                                             current = std::move(d);
                                         else
                                             m_upstream = std::vector{std::move(current), std::move(d)};
                                     },
                                     [&](std::vector<rpp::disposable_wrapper>& upstreams)
                                     {
                                         auto itr = std::find_if(upstreams.begin(),
                                                                 upstreams.end(),
                                                                 rpp::utils::static_mem_fn<&disposable_wrapper::is_disposed>{});
                                         if (itr != upstreams.cend())
                                             *itr = std::move(d);
                                         else
                                             upstreams.push_back(std::move(d));
                                     },
                                     [&](std::monostate) { m_upstream = std::move(d); }});
    }

    void dispose() const noexcept
    {
        visit(rpp::utils::overloaded{[](const rpp::disposable_wrapper& current) { current.dispose(); },
                                     [](const std::vector<rpp::disposable_wrapper>& upstreams)
                                     {
                                         for (auto& d : upstreams)
                                             d.dispose();
                                     },
                                     [](std::monostate) {}});
    }

    void clear() noexcept
    {
        visit(rpp::utils::overloaded{[](rpp::disposable_wrapper& current) { current = rpp::disposable_wrapper{}; },
                                     [](std::vector<rpp::disposable_wrapper>& upstreams) { upstreams.clear(); },
                                     [](std::monostate) {}});
    }

private:

    template<typename Fn>
    void visit(Fn&& fn)
    {
        if (m_upstream.valueless_by_exception())
            std::forward<Fn>(fn)(std::monostate{});
        else
            std::visit(std::forward<Fn>(fn), m_upstream);
    }

    template<typename Fn>
    void visit(Fn&& fn) const
    {
        if (m_upstream.valueless_by_exception())
            std::forward<Fn>(fn)(std::monostate{});
        else
            std::visit(std::forward<Fn>(fn), m_upstream);
    }

private:
    std::variant<std::monostate, rpp::disposable_wrapper, std::vector<disposable_wrapper>> m_upstream{};
};
} // namespace rpp::details