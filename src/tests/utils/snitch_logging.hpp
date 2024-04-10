#pragma once
#include <snitch/snitch_append.hpp>
#include <snitch/snitch_string.hpp>

#include <rpp/schedulers/fwd.hpp>

#include <tuple>
#include <vector>

template<typename T>
concept appendable = requires(snitch::small_string_span ss, const T& v) { append(ss, v); };

namespace rpp
{
    inline bool append(snitch::small_string_span ss, rpp::schedulers::time_point& v)
    {
        return append(ss, v.time_since_epoch().count());
    }
} // namespace rpp

namespace std
{
    template<appendable T>
    bool append(snitch::small_string_span ss, const std::vector<T>& v)
    {
        return append(ss, "{")
            && std::all_of(v.cbegin(), v.cend(), [&ss](const T& vv) { return append(ss, vv) && append(ss, ", "); })
            && append(ss, "}");
    }

    template<appendable... T>
    bool append(snitch::small_string_span ss, const std::tuple<T...>& v)
    {
        return append(ss, "{")
            && std::apply([&ss](const auto&... vv) { return ((append(ss, vv) && append(ss, ", ")) && ...); }, v)
            && append(ss, "}");
    }

    template<typename Clock, typename Duration>
    bool append(snitch::small_string_span ss, const std::chrono::time_point<Clock, Duration>& v)
    {
        return append(ss, v.time_since_epoch().count());
    }
} // namespace std
