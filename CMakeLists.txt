# MIT License
# 
# Copyright (c) 2021 Aleksey Loginov
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

project(ReactivePlusPlus VERSION 0.1 LANGUAGES CXX)

# ================== CONFIG ===================

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
find_package(Threads REQUIRED)

# ======================= OPTIONS ================================
set(BUILD_TESTS     OFF  CACHE BOOL "Enable unit tests building")
set(BUILD_SAMPLES   OFF  CACHE BOOL "Enable samples building")
set(CODE_COVERAGE   OFF CACHE BOOL "Enable coverage reporting")

# ======================= CI =========================
# Code Coverage Configuration
add_library(coverage_config INTERFACE)

if(CODE_COVERAGE)
  # Add required flags (GCC & LLVM/Clang)
  target_compile_options(coverage_config INTERFACE
    -O0        # no optimization
    -g         # generate debug info
    --coverage # sets all required flags
    -fprofile-arcs 
    -ftest-coverage
  )
  if(CMAKE_VERSION VERSION_GREATER_EQUAL 3.13)
    target_link_options(coverage_config INTERFACE --coverage)
  else()
    target_link_libraries(coverage_config INTERFACE --coverage)
  endif()
  target_link_libraries(coverage_config INTERFACE gcov)
endif(CODE_COVERAGE)

if (AUTOTESTS)
  Include(FetchContent)

  FetchContent_Declare(
    RxCpp
    GIT_REPOSITORY https://github.com/ReactiveX/RxCpp.git
    GIT_TAG        origin/main
  )

  FetchContent_MakeAvailable(RxCpp)
endif()

if(CMAKE_EXPORT_COMPILE_COMMANDS)
    set(CMAKE_CXX_STANDARD_INCLUDE_DIRECTORIES ${CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES})
endif()

# ================== TESTS ==================

if (BUILD_TESTS)
  enable_testing()
  set(TEST_RESULTS_DIR  ${CMAKE_BINARY_DIR}/test_results)

  macro (add_ctest target)
      add_test(NAME ${target} COMMAND $<TARGET_FILE:${target}> -r junit -o ${TEST_RESULTS_DIR}/${target}.xml)
  endmacro (add_ctest)

  file(MAKE_DIRECTORY ${TEST_RESULTS_DIR})
endif()

# ================== SOURCES =============

if (BUILD_TESTS)
  add_subdirectory(submodules)
endif()
add_subdirectory(src)
