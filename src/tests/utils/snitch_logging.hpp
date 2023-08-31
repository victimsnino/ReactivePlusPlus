#pragma once
#include <snitch/snitch_string.hpp>
#include <snitch/snitch_append.hpp>

#include <vector>

#include <sstream>

namespace std // NOLINT
{
bool append(snitch::small_string_span ss, const std::vector<rpp::schedulers::time_point>& v)
{
    std::stringstream res{};
    for (const auto& vv : v)
    {
        res << vv.time_since_epoch().count() << ", ";
    }
    return append(ss, res.str());
}

template<typename T>
    requires requires(const T& v) { std::stringstream{} << v; }
bool append(snitch::small_string_span ss, const std::vector<T>& v)
{
    std::stringstream res{};
    for (const auto& vv : v)
    {
        res << vv << ", ";
    }
    return append(ss, res.str());
}
} // namespace std