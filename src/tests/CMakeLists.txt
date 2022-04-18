#                  ReactivePlusPlus library
#
#          Copyright Aleksey Loginov 2022 - present.
# Distributed under the Boost Software License, Version 1.0.
#    (See accompanying file LICENSE_1_0.txt or copy at
#          https://www.boost.org/LICENSE_1_0.txt)
#
# Project home: https://github.com/victimsnino/ReactivePlusPlus
#

set(TARGET rpp_tests)

file(GLOB_RECURSE FILES "test_*.cpp")

add_executable(${TARGET} 
    ${FILES}

    mock_observer.h
    copy_count_tracker.h
)

target_link_libraries(${TARGET} PRIVATE rpp Catch2WithMain coverage_config)
set_target_properties(${TARGET} PROPERTIES FOLDER Tests)

add_ctest(${TARGET})

if(MSVC)
  target_compile_options(${TARGET} PRIVATE /W4 /WX)
else()
  target_compile_options(${TARGET} PRIVATE -Wall -Wextra -Wpedantic -Werror -Wconversion)
endif()
set(TARGET rpp_benchmark)

add_executable(${TARGET} 
    benchmarks.cpp
)

target_link_libraries(${TARGET} PRIVATE rpp Catch2WithMain)
set_target_properties(${TARGET} PROPERTIES FOLDER Tests)


if (AUTOTESTS)
  set(TARGET rxcpp_benchmark)

  add_executable(${TARGET} 
      rxcpp_benchmark.cpp
  )

  target_link_libraries(${TARGET} PRIVATE rxcpp Catch2WithMain)
  set_target_properties(${TARGET} PROPERTIES FOLDER Tests)
endif()