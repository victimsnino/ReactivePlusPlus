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

project(ReactivePlusPlus VERSION 0.1 LANGUAGES C CXX)

# ================== CONFIG ===================
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

find_package(Threads REQUIRED)

# ======================= CI =========================
set(TEST_RESULTS_DIR  ${CMAKE_BINARY_DIR}/test_results)
set(BUILD_TESTS ON CACHE BOOL "Enable unit tests building" FORCE)

# Code Coverage Configuration
add_library(coverage_config INTERFACE)

option(CODE_COVERAGE "Enable coverage reporting" OFF)
if(CODE_COVERAGE)
  # Add required flags (GCC & LLVM/Clang)
  target_compile_options(coverage_config INTERFACE
    -O0        # no optimization
    -g         # generate debug info
    --coverage # sets all required flags
    -fprofile-arcs 
    -ftest-coverage
    -fprofile-instr-generate 
    -fcoverage-mapping
  )
  if(CMAKE_VERSION VERSION_GREATER_EQUAL 3.13)
    target_link_options(coverage_config INTERFACE --coverage)
  else()
    target_link_libraries(coverage_config INTERFACE --coverage)
  endif()
  target_link_libraries(coverage_config INTERFACE gcov)
  
  SET(CMAKE_CXX_FLAGS "-g -O0 --coverage -fprofile-arcs -ftest-coverage")
  SET(CMAKE_C_FLAGS "-g -O0 --coverage -fprofile-arcs -ftest-coverage")

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


# ================== TESTS ==================
enable_testing()

macro (add_ctest target)
  add_test(NAME ${target} COMMAND $<TARGET_FILE:${target}> -r junit -o ${TEST_RESULTS_DIR}/${target}.xml ~[benchmark] ~[misc_benchmark])
endmacro (add_ctest)

file(MAKE_DIRECTORY ${TEST_RESULTS_DIR})

# ================== SOURCES =============

add_subdirectory(submodules)
add_subdirectory(src)
