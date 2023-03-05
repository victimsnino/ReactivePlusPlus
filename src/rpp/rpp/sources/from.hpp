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

#include <rpp/sources/fwd.hpp>
#include <rpp/observables/interface_observable.hpp>
#include <rpp/utils/utils.hpp>

namespace rpp::source::details
{
template<constraint::decayed_type Container>
class shared_container
{
public:
    template<typename ...Ts>
    shared_container(Ts&&...items)
        // raw "new" call to avoid extra copy/moves for items
        : m_container{new Container{std::forward<Ts>(items)...}} {}

    auto begin() const { return std::begin(*m_container); }
    auto end() const { return std::end(*m_container); }

private:
    std::shared_ptr<Container> m_container{};
};

template<memory_model memory_model, constraint::iterable Container, typename ...Ts>
auto pack_to_container(Ts&& ...items)
{
    if constexpr (memory_model == memory_model::use_stack)
        return Container{std::forward<Ts>(items)...};
    else
        return shared_container<Container>{std::forward<Ts>(items)...};
}

template<memory_model memory_model, constraint::decayed_type T, typename ...Ts>
auto pack_variadic(Ts&& ...items)
{
    return pack_to_container<memory_model, std::array<T, sizeof...(Ts)>>(std::forward<Ts>(items)...);
}
} // namespace rpp::source::details

namespace rpp::source
{
template<constraint::decayed_type PackedContainer>
class iterate_observabe final : public interface_observable<utils::iterable_value_t<PackedContainer>>
{
public:
    iterate_observabe(const PackedContainer& container)
        : m_container{container} {}

    iterate_observabe(PackedContainer&& container)
        : m_container{std::move(container)} {}

protected:
    composite_disposable subscribe_impl(const interface_observer<utils::iterable_value_t<PackedContainer>>& observer) const override
    {
        for (const auto& v : m_container)
            observer.on_next(v);
        observer.on_completed();

        return composite_disposable::empty();
    }
private:
    PackedContainer m_container;
};

template<memory_model memory_model/* = memory_model::use_stack*,/ /* schedulers::constraint::scheduler TScheduler = schedulers::trampoline*/>
auto from_iterable(constraint::iterable auto&& iterable/*, const TScheduler& scheduler = TScheduler{}*/)
// requires rpp::details::is_header_included <rpp::details::from_tag, TScheduler > {
{
    return iterate_observabe{ details::pack_to_container<memory_model, std::decay_t<decltype(iterable)>>(std::forward<decltype(iterable)>(iterable))};
}
} // namespace rpp::source
