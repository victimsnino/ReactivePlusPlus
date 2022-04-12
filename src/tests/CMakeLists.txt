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

set(TARGET rpp_tests)

add_executable(${TARGET} 
    test_observable.cpp
    test_observer.cpp
    test_subscriber.cpp
    test_map.cpp
    test_schedulers.cpp
    test_subscriptions.cpp
    test_filter.cpp
    
    test_lift.cpp

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