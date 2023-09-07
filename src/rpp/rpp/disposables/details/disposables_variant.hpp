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

#include <variant>
#include <vector>

namespace rpp::details
{
class disposables_variant
{
public:
    disposables_variant() = default;

    void add(rpp::disposable_wrapper&& d)
    {
        if (const auto current_ptr = std::get_if<rpp::disposable_wrapper>(&m_upstream))
        {
            if (current_ptr->is_disposed())
                *current_ptr = std::move(d);
            else
                m_upstream = std::vector{std::move(*current_ptr), std::move(d)};
        }
        else if(const auto upstreams = std::get_if<std::vector<rpp::disposable_wrapper>>(&m_upstream))
        {
            auto itr = std::find_if(upstreams->begin(), upstreams->end(), rpp::utils::static_mem_fn<&disposable_wrapper::is_disposed>{});
            if (itr != upstreams->cend())
                *itr = std::move(d);
            else
                upstreams->push_back(std::move(d));
        }
        else
            m_upstream = std::move(d);
    }

    void dispose() const noexcept
    {
        if (const auto current_ptr = std::get_if<rpp::disposable_wrapper>(&m_upstream))
        {
            current_ptr->dispose();
        }
        else if (const auto upstreams = std::get_if<std::vector<rpp::disposable_wrapper>>(&m_upstream))
        {
            for (const auto& d : *upstreams)
                d.dispose();
        }
    }

    void clear() noexcept
    {
        // if (const auto current_ptr = std::get_if<rpp::disposable_wrapper>(&m_upstream))
        //     *current_ptr = rpp::disposable_wrapper{};
        // else if (const auto upstreams = std::get_if<std::vector<rpp::disposable_wrapper>>(&m_upstream))
        //     upstreams->clear();
    }

private:
    std::variant<std::monostate, rpp::disposable_wrapper, std::vector<disposable_wrapper>> m_upstream{};
};
} // namespace rpp::details