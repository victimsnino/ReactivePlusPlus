#                  ReactivePlusPlus library
#
#          Copyright Aleksey Loginov 2022 - present.
# Distributed under the Boost Software License, Version 1.0.
#    (See accompanying file LICENSE_1_0.txt or copy at
#          https://www.boost.org/LICENSE_1_0.txt)
#
# Project home: https://github.com/victimsnino/ReactivePlusPlus
#

file(GLOB_RECURSE FILES "test_*.cpp")

foreach(SOURCE ${FILES}) 
  get_filename_component(BASE_NAME ${SOURCE} NAME_WE)

  set(TARGET ${BASE_NAME})

  add_executable(${TARGET} ${SOURCE} mock_observer.hpp copy_count_tracker.hpp)
  target_link_libraries(${TARGET} PRIVATE rpp Catch2WithMain coverage_config)
  set_target_properties(${TARGET} PROPERTIES FOLDER Tests/Suites)

  add_ctest(${TARGET})

  if(MSVC)
    target_compile_options(${TARGET} PRIVATE /W4 /WX)
  else()
    target_compile_options(${TARGET} PRIVATE -Wall -Wextra -Wpedantic -Werror -Wconversion)
  endif()

endforeach()

# ================ BENCHMARKS =================
set(TARGET rpp_benchmark)

add_executable(${TARGET} 
    benchmarks.cpp
)

target_link_libraries(${TARGET} PRIVATE rpp Catch2WithMain)
set_target_properties(${TARGET} PROPERTIES FOLDER Tests)


if (RPP_AUTOTESTS)
  set(TARGET rxcpp_benchmark)

  add_executable(${TARGET} 
      rxcpp_benchmark.cpp
  )

  target_link_libraries(${TARGET} PRIVATE rxcpp Catch2WithMain)
  set_target_properties(${TARGET} PROPERTIES FOLDER Tests)
endif()