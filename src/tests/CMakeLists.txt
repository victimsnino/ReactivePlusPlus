#                  ReactivePlusPlus library
#
#          Copyright Aleksey Loginov 2022 - present.
# Distributed under the Boost Software License, Version 1.0.
#    (See accompanying file LICENSE_1_0.txt or copy at
#          https://www.boost.org/LICENSE_1_0.txt)
#
# Project home: https://github.com/victimsnino/ReactivePlusPlus
#

# ====================== Gather tests ======================
file(GLOB_RECURSE FILES "test_*.cpp")

foreach(SOURCE ${FILES}) 
  get_filename_component(BASE_NAME ${SOURCE} NAME_WE)

  set(TARGET ${BASE_NAME})

  add_executable(${TARGET} ${SOURCE} mock_observer.hpp copy_count_tracker.hpp test_scheduler.hpp)
  target_link_libraries(${TARGET} PRIVATE rpp Catch2::Catch2WithMain)
  set_target_properties(${TARGET} PROPERTIES FOLDER Tests/Suites)

  add_test(NAME ${TARGET} COMMAND $<TARGET_FILE:${TARGET}> -r junit -o ${RPP_TEST_RESULTS_DIR}/${TARGET}.xml)

  if(MSVC)
    target_compile_options(${TARGET} PRIVATE /W4 /WX)
  else()
    target_compile_options(${TARGET} PRIVATE -Wall -Wextra -Wpedantic -Werror -Wconversion)
  endif()

endforeach()

# ================ BENCHMARKS =================

add_custom_target(run_rpp_ci_benchmarks)

set(TARGET rpp_benchmark)

add_executable(${TARGET} 
    benchmarks.cpp
)

target_link_libraries(${TARGET} PRIVATE rpp Catch2::Catch2WithMain)
set_target_properties(${TARGET} PROPERTIES FOLDER Tests)

add_custom_command(TARGET run_rpp_ci_benchmarks COMMAND $<TARGET_FILE:${TARGET}> -r xml -o ${RPP_TEST_BENCHMARKS_RESULTS_DIR}/${TARGET}_result.txt COMMENT "Running benchmark ${TARGET}...")


if (RPP_BUILD_RXCPP)
  set(TARGET rxcpp_benchmark)

  add_executable(${TARGET} 
      rxcpp_benchmark.cpp
  )

  target_link_libraries(${TARGET} PRIVATE rxcpp Catch2::Catch2WithMain)
  get_target_property(RXCPP_DIR rxcpp INTERFACE_INCLUDE_DIRECTORIES)
  target_include_directories(${TARGET} SYSTEM PRIVATE ${RXCPP_DIR})
  set_target_properties(${TARGET} PROPERTIES FOLDER Tests)

  add_custom_command(TARGET run_rpp_ci_benchmarks COMMAND $<TARGET_FILE:${TARGET}> -r xml -o ${RPP_TEST_BENCHMARKS_RESULTS_DIR}/${TARGET}_result.txt COMMENT "Running benchmark ${TARGET}...")
endif()
