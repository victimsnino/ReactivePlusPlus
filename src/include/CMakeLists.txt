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

file(GLOB_RECURSE FILES "*.h")

add_library(rpp INTERFACE ${FILES})
target_sources(rpp INTERFACE ${FILES})
target_include_directories(rpp INTERFACE .)
target_link_libraries(rpp INTERFACE coverage_config)

foreach(FILE ${FILES}) 
  get_filename_component(PARENT_DIR "${FILE}" PATH)
  file(RELATIVE_PATH REL_PARENT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/rpp" ${PARENT_DIR})
  set(REL_PARENT_DIR "Header Files\\${REL_PARENT_DIR}")

  string(REPLACE "/" "\\" GROUP ${REL_PARENT_DIR})
  source_group("${GROUP}" FILES "${FILE}")
endforeach()
