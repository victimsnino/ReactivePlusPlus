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

  find_program(CCACHE ccache)
  if(CCACHE)
      set(CMAKE_C_COMPILER_LAUNCHER ${CCACHE})
      set(CMAKE_CXX_COMPILER_LAUNCHER ${CCACHE})
  endif(CCACHE)
endif()


function(rpp_add_library NAME)
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

  file(GLOB_RECURSE FILES "*.hpp")

  if(${CMAKE_VERSION} VERSION_LESS "3.19.0")
    add_library(${NAME} INTERFACE)
  else()
    add_library(${NAME} INTERFACE ${FILES})
  endif()

  add_library(RPP::${NAME} ALIAS ${NAME})

  target_include_directories(${NAME} ${RPP_WARNING_GUARD}
    INTERFACE
      "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
      # "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
  )

  target_link_libraries(${NAME} INTERFACE Threads::Threads)
  if (NOT ${NAME} STREQUAL "rpp")
    target_link_libraries(${NAME} INTERFACE RPP::rpp)
  endif()

  target_compile_features(${NAME} INTERFACE cxx_std_20)

  if (CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    target_compile_options(${NAME} INTERFACE -fsized-deallocation)
  endif()

  foreach(FILE ${FILES})
    get_filename_component(PARENT_DIR "${FILE}" PATH)
    file(RELATIVE_PATH REL_PARENT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/${NAME}" ${PARENT_DIR})
    set(REL_PARENT_DIR "Header Files\\${REL_PARENT_DIR}")

    string(REPLACE "/" "\\" GROUP ${REL_PARENT_DIR})
    source_group("${GROUP}" FILES "${FILE}")
  endforeach()
endfunction()


# ------------ Options to tweak ---------------------
option(RPP_BUILD_SFML_CODE "Enable SFML support in examples/code." OFF)
option(RPP_BUILD_QT_CODE "Enable QT support in examples/code." OFF)

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
