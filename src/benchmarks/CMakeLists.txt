#                  ReactivePlusPlus library
#
#          Copyright Aleksey Loginov 2022 - present.
# Distributed under the Boost Software License, Version 1.0.
#    (See accompanying file LICENSE_1_0.txt or copy at
#          https://www.boost.org/LICENSE_1_0.txt)
#
# Project home: https://github.com/victimsnino/ReactivePlusPlus
#


function(add_benchmark_extended benchmark_name library)
  set(TARGET ${benchmark_name}_benchmark)

  add_executable(${TARGET} ${TARGET}.cpp)

  target_link_libraries(${TARGET} PRIVATE ${library} Catch2::Catch2WithMain)
  set_target_properties(${TARGET} PROPERTIES FOLDER Tests)

  add_test(NAME ${TARGET} COMMAND  $<TARGET_FILE:${TARGET}> -r xml -o ${RPP_TEST_BENCHMARKS_RESULTS_DIR}/${TARGET}_result.txt)

  if (NOT library STREQUAL "rpp")
    get_target_property(DEP_DIR ${library} INTERFACE_INCLUDE_DIRECTORIES)
    target_include_directories(${TARGET} SYSTEM PRIVATE ${DEP_DIR})
  endif()

endfunction()

function(add_benchmark benchmark_name)
  add_benchmark_extended(${benchmark_name} ${benchmark_name})
endfunction()

add_benchmark(rpp)
add_benchmark_extended(rpp_type_erased rpp)


if (RPP_BUILD_RXCPP)
  add_benchmark(rxcpp)
endif()