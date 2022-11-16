#                  ReactivePlusPlus library
#
#          Copyright Aleksey Loginov 2022 - present.
# Distributed under the Boost Software License, Version 1.0.
#    (See accompanying file LICENSE_1_0.txt or copy at
#          https://www.boost.org/LICENSE_1_0.txt)
#
# Project home: https://github.com/victimsnino/ReactivePlusPlus
#

file(GLOB_RECURSE FILES "*.hpp")

if(${CMAKE_VERSION} VERSION_LESS "3.19.0")
  add_library(rppqt INTERFACE)
else()
  add_library(rppqt INTERFACE ${FILES})
endif()

add_library(RPP::rppqt ALIAS rppqt)

target_include_directories(rppqt ${RPP_WARNING_GUARD} 
  INTERFACE 
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
    # "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
)

target_link_libraries(rppqt INTERFACE Threads::Threads RPP::rpp)
target_compile_features(rppqt INTERFACE cxx_std_20)

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_ID MATCHES "GNU")
  target_compile_options(rppqt INTERFACE -fsized-deallocation)
endif()

foreach(FILE ${FILES}) 
  get_filename_component(PARENT_DIR "${FILE}" PATH)
  file(RELATIVE_PATH REL_PARENT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/rppqt" ${PARENT_DIR})
  set(REL_PARENT_DIR "Header Files\\${REL_PARENT_DIR}")

  string(REPLACE "/" "\\" GROUP ${REL_PARENT_DIR})
  source_group("${GROUP}" FILES "${FILE}")
endforeach()
