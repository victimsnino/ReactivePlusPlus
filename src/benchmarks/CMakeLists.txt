#                  ReactivePlusPlus library
#
#          Copyright Aleksey Loginov 2023 - present.
# Distributed under the Boost Software License, Version 1.0.
#    (See accompanying file LICENSE_1_0.txt or copy at
#          https://www.boost.org/LICENSE_1_0.txt)
#
# Project home: https://github.com/victimsnino/ReactivePlusPlus
#

set(TARGET benchmarks)

add_executable(${TARGET} benchmarks.cpp)

target_link_libraries(${TARGET} PRIVATE rpp nanobench)
set_target_properties(${TARGET} PROPERTIES FOLDER Tests)

add_test(NAME ${TARGET} COMMAND  $<TARGET_FILE:${TARGET}> -r xml -o ${RPP_TEST_BENCHMARKS_RESULTS_DIR}/${TARGET}_result.txt)
