#pragma once

#include <snitch/snitch_macros_check.hpp>
#include <snitch/snitch_macros_misc.hpp>

#include <trompeloeil.hpp>

#include <rpp/disposables/fwd.hpp>
#include <exception>


namespace trompeloeil
{
  template <>
  inline void reporter<specialized>::send(
    severity s,
    const char* file,
    unsigned long line,
    const char* msg)
  {
    std::ostringstream os;
    if (line) os << file << ':' << line << '\n';
    os << msg;
    auto failure = os.str();
    if (s == severity::fatal)
    {
      FAIL(failure);
    }
    else
    {
      CAPTURE(failure);
      CHECK(failure.empty());
    }
  }

  template <>
  inline void reporter<specialized>::sendOk(
    const char* trompeloeil_mock_calls_done_correctly)
  {      
      REQUIRE(trompeloeil_mock_calls_done_correctly != 0);
  }
}

template<typename T>
struct mock_observer
{
    static constexpr bool trompeloeil_movable_mock = true;

    MAKE_MOCK1(on_next, void(const T&), const);
    MAKE_MOCK1(on_next, void(T&&), const);
    MAKE_MOCK1(on_error, void(const std::exception_ptr& err), const);
    MAKE_MOCK0(on_completed, void(), const);

    static bool is_disposed() noexcept { return false; }
    static void set_upstream(const rpp::disposable_wrapper&) noexcept {}
};