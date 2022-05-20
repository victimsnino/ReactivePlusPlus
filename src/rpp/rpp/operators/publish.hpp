#pragma once

#include <rpp/observables/constraints.hpp>
#include <rpp/operators/fwd/publish.hpp>
#include <rpp/operators/multicast.hpp>

IMPLEMENTATION_FILE(publish_tag);

namespace rpp::details
{
template<constraint::decayed_type Type, constraint::observable_of_type<Type> TObs>
auto publish_impl(TObs&& observable)
{
    return std::forward<TObs>(observable).multicast(rpp::subjects::publish_subject<Type>{});
}
} // namespace rpp::details
