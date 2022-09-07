# ---- Developer mode ----

# Developer mode enables targets and code paths in the CMake scripts that are
# only relevant for the developer(s) of RPP
# Targets necessary to build the project must be provided unconditionally, so
# consumers can trivially build and package the project
if(PROJECT_IS_TOP_LEVEL)
  option(RPP_DEVELOPER_MODE "Enable developer mode" OFF)

  set(CMAKE_CXX_STANDARD 20)
  set(CMAKE_CXX_STANDARD_REQUIRED ON)
  set(CMAKE_CXX_EXTENSIONS OFF)

  set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

  set_property(GLOBAL PROPERTY USE_FOLDERS ON)
endif()


# ---- Warning guard ----

# target_include_directories with the SYSTEM modifier will request the compiler
# to omit warnings from the provided paths, if the compiler supports that
# This is to provide a user experience similar to find_package when
# add_subdirectory or FetchContent is used to consume this project
set(RPP_WARNING_GUARD "")
if(NOT PROJECT_IS_TOP_LEVEL)
  option(RPP_INCLUDES_WITH_SYSTEM "Use SYSTEM modifier for RPP's includes, disabling warnings" ON)
  mark_as_advanced(RPP_INCLUDES_WITH_SYSTEM)
  if(RPP_INCLUDES_WITH_SYSTEM)
    set(RPP_WARNING_GUARD SYSTEM)
  endif()
endif()


# ------------ Options to tweak ---------------------
option(RPP_BUILD_SFML_CODE        "Enable SFML support in examples/code." OFF)
option(RPP_TYPE_ERASED_OBSERVABLE "force observable to store any `on_subscribe` as `std::function`. As a result it reduces amount of uniq observables, speedups compilation and IDE loading time a lot, but has a major performance hit. Strongly recommended to avoid this option. Enable ONLY when it is not possible to use RPP as is (for example, system has limits over ROM/RAM/CPU)" OFF)

if (RPP_DEVELOPER_MODE)
  option(RPP_BUILD_TESTS      "Build unit tests tree." OFF)
  option(RPP_BUILD_BENCHMARKS "Build benchmarks tree." OFF)
  option(RPP_BUILD_EXAMPLES   "Build examples tree." OFF)
  option(RPP_ENABLE_COVERAGE  "Enable coverage support separate from CTest's" OFF)
  option(RPP_BUILD_RXCPP      "Build RxCpp to compare results with it." OFF)

  if(RPP_ENABLE_COVERAGE)
    include(cmake/coverage.cmake)
  endif()
endif()