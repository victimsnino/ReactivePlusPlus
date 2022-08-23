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

cmake_minimum_required(VERSION 3.12)

if(DEFINED PROJECT_NAME)
  set(IS_SUBPROJECT ON)
else()
  set(IS_SUBPROJECT OFF)
endif()

project(ReactivePlusPlus VERSION 0.0.6 LANGUAGES CXX)

# ================== CONFIG ===================

if (NOT IS_SUBPROJECT)
  set(CMAKE_CXX_STANDARD 20)
  set(CMAKE_CXX_STANDARD_REQUIRED ON)
  set(CMAKE_CXX_EXTENSIONS OFF)

  set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

  set_property(GLOBAL PROPERTY USE_FOLDERS ON)
endif()

# ======================= OPTIONS ================================
set(RPP_BUILD_TESTS     OFF CACHE BOOL "Enable unit tests building")
set(RPP_BUILD_SAMPLES   OFF CACHE BOOL "Enable samples building")
set(RPP_CODE_COVERAGE   OFF CACHE BOOL "Enable coverage reporting")
set(RPP_BUILD_SFML_CODE OFF CACHE BOOL "Enable SFML support in samples/code")

include(CMake/dependencies.cmake)
include(CMake/coverage_config.cmake)

if (RPP_BUILD_TESTS)
  enable_testing()
  set(RPP_TEST_RESULTS_DIR  ${CMAKE_BINARY_DIR}/test_results)
  file(MAKE_DIRECTORY ${RPP_TEST_RESULTS_DIR})
endif()

add_subdirectory(src)

include(CMake/install.cmake)