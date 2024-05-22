#pragma once

#include <snitch/snitch_macros_check.hpp>
#include <snitch/snitch_macros_misc.hpp>

#include <trompeloeil.hpp>

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