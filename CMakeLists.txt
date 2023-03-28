# MIT License
#
# Copyright (c) 2022 Aleksey Loginov
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
#
cmake_minimum_required(VERSION 3.14)

include(cmake/prelude.cmake)

project(
    ReactivePlusPlus
    VERSION 0.2.0
    DESCRIPTION "ReactivePlusPlus is library for building asynchronous event-driven streams of data with help of sequences of primitive operators in the declarative form"
    HOMEPAGE_URL "https://github.com/victimsnino/ReactivePlusPlus"
    LANGUAGES CXX
)

include(cmake/project-is-top-level.cmake)
include(cmake/variables.cmake)
include(cmake/dependencies.cmake)

if (RPP_BUILD_TESTS OR RPP_BUILD_BENCHMARKS)
  enable_testing()
  set(RPP_TEST_RESULTS_DIR  ${CMAKE_BINARY_DIR}/test_results)
  file(MAKE_DIRECTORY ${RPP_TEST_RESULTS_DIR})

  if (RPP_BUILD_BENCHMARKS)
    set(RPP_TEST_BENCHMARKS_RESULTS_DIR  ${RPP_TEST_RESULTS_DIR}/benchmarks)
    file(MAKE_DIRECTORY ${RPP_TEST_BENCHMARKS_RESULTS_DIR})
  endif()
endif()

add_subdirectory(src)

# ---- Install rules ----

if(NOT CMAKE_SKIP_INSTALL_RULES)
  include(cmake/install-rules.cmake)
endif()

# ---- Developer mode ----

if(NOT RPP_DEVELOPER_MODE)
  return()
endif()

if(NOT PROJECT_IS_TOP_LEVEL)
  message(AUTHOR_WARNING "RPP_DEVELOPER_MODE is intended for developers of ReactivePlusPlus")
endif()

include(cmake/dev-mode.cmake)
