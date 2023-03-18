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
#include "rpp/utils/constraints.hpp"
#include <concepts>
#include <exception>
#include <functional>
#include <rpp/observers/interface_observer.hpp>
#include <rpp/observers/dynamic_observer.hpp>

#include <variant>

namespace rpp::details
{
template<constraint::decayed_type T>
using variant_observer = std::variant<std::reference_wrapper<interface_observer<T>>, dynamic_observer<T>>;

template <typename S, typename Type, typename DownStreamType>
concept operator_observer_strategy = std::copy_constructible<S> &&
    requires(const S s, const Type &v,
             const interface_observer<DownStreamType> &obs,
             const composite_disposable &disposable) {
  s.on_next(obs, v);
  s.on_error(obs, std::exception_ptr{});
  s.on_completed(obs);
} && requires(S s, interface_observer<DownStreamType> &obs,
              const composite_disposable &disposable) {
  s.set_resource(disposable, obs);
};

struct forwarding_on_next_strategy {
    template<constraint::decayed_type DownStreamType>
    static void on_next(const interface_observer<DownStreamType>& obs, const DownStreamType& v) noexcept { obs.on_next(v); }

    template<constraint::decayed_type DownStreamType>
    static void on_next(const interface_observer<DownStreamType>& obs, DownStreamType&& v) noexcept { obs.on_next(std::move(v)); }
};

struct forwarding_on_error_strategy {
    template<constraint::decayed_type DownStreamType>
    static void on_error(const interface_observer<DownStreamType>& obs, const std::exception_ptr& err) noexcept { obs.on_error(err); }
};

struct forwarding_on_completed_strategy {
    template<constraint::decayed_type DownStreamType>
    static void on_completed(const interface_observer<DownStreamType>& obs) noexcept { obs.on_completed(); }
};

template<constraint::decayed_type Type, constraint::decayed_type DownStreamType, operator_observer_strategy<Type, DownStreamType> Strategy>
class operator_observer final : public interface_observer<Type>
{
public:
    operator_observer(const Strategy& strategy, variant_observer<DownStreamType> downstream)
        : m_downstream{std::move(downstream)}
        , m_strategy{strategy} {}

    operator_observer(Strategy&& strategy, variant_observer<DownStreamType> downstream)
        : m_downstream{std::move(downstream)}
        , m_strategy{std::move(strategy)} {}

    operator_observer(const operator_observer&) = delete;
    operator_observer(operator_observer&&) noexcept = default;

    void on_next(const Type& v) const noexcept override                  { std::visit([&v, this](const interface_observer<DownStreamType>& obs) { m_strategy.on_next(obs, v); },            m_downstream); }
    void on_next(Type&& v) const noexcept override                       { std::visit([&v, this](const interface_observer<DownStreamType>& obs) { m_strategy.on_next(obs, std::move(v)); }, m_downstream); }
    void on_error(const std::exception_ptr& err) const noexcept override { std::visit([err, this](const interface_observer<DownStreamType>& obs){ m_strategy.on_error(obs, err); },         m_downstream); }
    void on_completed() const noexcept override                          { std::visit([this](const interface_observer<DownStreamType>& obs)     { m_strategy.on_completed(obs); },          m_downstream); }

    bool is_disposed() const noexcept override
    {
        return std::visit(&interface_observer<DownStreamType>::is_disposed, m_downstream);
    }

    void set_resource(const composite_disposable& disposable) noexcept override
    {
        std::visit([this, &disposable](interface_observer<DownStreamType>& obs) { m_strategy.set_resource(disposable, obs); }, m_downstream);
    }

    dynamic_observer<Type> as_dynamic() &&noexcept override {
      return {std::make_shared<operator_observer>(
          std::move(m_strategy),
          std::visit([](interface_observer<DownStreamType>& obs)
          {
                return std::move(obs).as_dynamic();
          },
          m_downstream))};
    }

private:
    variant_observer<DownStreamType> m_downstream;
    [[no_unique_address]] Strategy   m_strategy;
};
} // namespace rpp::details
