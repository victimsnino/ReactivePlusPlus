#                  ReactivePlusPlus library
#
#          Copyright Aleksey Loginov 2022 - present.
# Distributed under the Boost Software License, Version 1.0.
#    (See accompanying file LICENSE_1_0.txt or copy at
#          https://www.boost.org/LICENSE_1_0.txt)
#
# Project home: https://github.com/victimsnino/ReactivePlusPlus
#


function(add_benchmark benchmark_name)
  set(TARGET ${benchmark_name}_benchmark)

  add_executable(${TARGET} ${TARGET}.cpp)

  target_link_libraries(${TARGET} PRIVATE ${benchmark_name} Catch2::Catch2WithMain)
  set_target_properties(${TARGET} PROPERTIES FOLDER Tests)
	@@ -26,7 +26,14 @@ function(add_benchmark benchmark_name)

endfunction()

add_benchmark(rpp)
add_benchmark(rpp_type_erased)


if (RPP_BUILD_RXCPP)
  add_benchmark(rxcpp)
endif()