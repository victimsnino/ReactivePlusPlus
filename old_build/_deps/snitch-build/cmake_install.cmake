# Install script for directory: /home/victi/ReactivePlusPlus/build/_deps/snitch-src

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr/local")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Install shared libraries without execute permission?
if(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  set(CMAKE_INSTALL_SO_NO_EXE "1")
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

# Set default install directory permissions.
if(NOT DEFINED CMAKE_OBJDUMP)
  set(CMAKE_OBJDUMP "/usr/bin/objdump")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/usr/local/include/snitch/snitch.hpp;/usr/local/include/snitch/snitch_append.hpp;/usr/local/include/snitch/snitch_capture.hpp;/usr/local/include/snitch/snitch_cli.hpp;/usr/local/include/snitch/snitch_concepts.hpp;/usr/local/include/snitch/snitch_console.hpp;/usr/local/include/snitch/snitch_error_handling.hpp;/usr/local/include/snitch/snitch_expression.hpp;/usr/local/include/snitch/snitch_file.hpp;/usr/local/include/snitch/snitch_fixed_point.hpp;/usr/local/include/snitch/snitch_function.hpp;/usr/local/include/snitch/snitch_macros_check.hpp;/usr/local/include/snitch/snitch_macros_check_base.hpp;/usr/local/include/snitch/snitch_macros_consteval.hpp;/usr/local/include/snitch/snitch_macros_constexpr.hpp;/usr/local/include/snitch/snitch_macros_exceptions.hpp;/usr/local/include/snitch/snitch_macros_misc.hpp;/usr/local/include/snitch/snitch_macros_reporter.hpp;/usr/local/include/snitch/snitch_macros_test_case.hpp;/usr/local/include/snitch/snitch_macros_utility.hpp;/usr/local/include/snitch/snitch_macros_warnings.hpp;/usr/local/include/snitch/snitch_matcher.hpp;/usr/local/include/snitch/snitch_registry.hpp;/usr/local/include/snitch/snitch_reporter_catch2_xml.hpp;/usr/local/include/snitch/snitch_reporter_console.hpp;/usr/local/include/snitch/snitch_reporter_teamcity.hpp;/usr/local/include/snitch/snitch_section.hpp;/usr/local/include/snitch/snitch_string.hpp;/usr/local/include/snitch/snitch_string_utility.hpp;/usr/local/include/snitch/snitch_test_data.hpp;/usr/local/include/snitch/snitch_type_name.hpp;/usr/local/include/snitch/snitch_vector.hpp;/usr/local/include/snitch/snitch_config.hpp")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  file(INSTALL DESTINATION "/usr/local/include/snitch" TYPE FILE FILES
    "/home/victi/ReactivePlusPlus/build/_deps/snitch-src/include/snitch/snitch.hpp"
    "/home/victi/ReactivePlusPlus/build/_deps/snitch-src/include/snitch/snitch_append.hpp"
    "/home/victi/ReactivePlusPlus/build/_deps/snitch-src/include/snitch/snitch_capture.hpp"
    "/home/victi/ReactivePlusPlus/build/_deps/snitch-src/include/snitch/snitch_cli.hpp"
    "/home/victi/ReactivePlusPlus/build/_deps/snitch-src/include/snitch/snitch_concepts.hpp"
    "/home/victi/ReactivePlusPlus/build/_deps/snitch-src/include/snitch/snitch_console.hpp"
    "/home/victi/ReactivePlusPlus/build/_deps/snitch-src/include/snitch/snitch_error_handling.hpp"
    "/home/victi/ReactivePlusPlus/build/_deps/snitch-src/include/snitch/snitch_expression.hpp"
    "/home/victi/ReactivePlusPlus/build/_deps/snitch-src/include/snitch/snitch_file.hpp"
    "/home/victi/ReactivePlusPlus/build/_deps/snitch-src/include/snitch/snitch_fixed_point.hpp"
    "/home/victi/ReactivePlusPlus/build/_deps/snitch-src/include/snitch/snitch_function.hpp"
    "/home/victi/ReactivePlusPlus/build/_deps/snitch-src/include/snitch/snitch_macros_check.hpp"
    "/home/victi/ReactivePlusPlus/build/_deps/snitch-src/include/snitch/snitch_macros_check_base.hpp"
    "/home/victi/ReactivePlusPlus/build/_deps/snitch-src/include/snitch/snitch_macros_consteval.hpp"
    "/home/victi/ReactivePlusPlus/build/_deps/snitch-src/include/snitch/snitch_macros_constexpr.hpp"
    "/home/victi/ReactivePlusPlus/build/_deps/snitch-src/include/snitch/snitch_macros_exceptions.hpp"
    "/home/victi/ReactivePlusPlus/build/_deps/snitch-src/include/snitch/snitch_macros_misc.hpp"
    "/home/victi/ReactivePlusPlus/build/_deps/snitch-src/include/snitch/snitch_macros_reporter.hpp"
    "/home/victi/ReactivePlusPlus/build/_deps/snitch-src/include/snitch/snitch_macros_test_case.hpp"
    "/home/victi/ReactivePlusPlus/build/_deps/snitch-src/include/snitch/snitch_macros_utility.hpp"
    "/home/victi/ReactivePlusPlus/build/_deps/snitch-src/include/snitch/snitch_macros_warnings.hpp"
    "/home/victi/ReactivePlusPlus/build/_deps/snitch-src/include/snitch/snitch_matcher.hpp"
    "/home/victi/ReactivePlusPlus/build/_deps/snitch-src/include/snitch/snitch_registry.hpp"
    "/home/victi/ReactivePlusPlus/build/_deps/snitch-src/include/snitch/snitch_reporter_catch2_xml.hpp"
    "/home/victi/ReactivePlusPlus/build/_deps/snitch-src/include/snitch/snitch_reporter_console.hpp"
    "/home/victi/ReactivePlusPlus/build/_deps/snitch-src/include/snitch/snitch_reporter_teamcity.hpp"
    "/home/victi/ReactivePlusPlus/build/_deps/snitch-src/include/snitch/snitch_section.hpp"
    "/home/victi/ReactivePlusPlus/build/_deps/snitch-src/include/snitch/snitch_string.hpp"
    "/home/victi/ReactivePlusPlus/build/_deps/snitch-src/include/snitch/snitch_string_utility.hpp"
    "/home/victi/ReactivePlusPlus/build/_deps/snitch-src/include/snitch/snitch_test_data.hpp"
    "/home/victi/ReactivePlusPlus/build/_deps/snitch-src/include/snitch/snitch_type_name.hpp"
    "/home/victi/ReactivePlusPlus/build/_deps/snitch-src/include/snitch/snitch_vector.hpp"
    "/home/victi/ReactivePlusPlus/build/_deps/snitch-build/snitch/snitch_config.hpp"
    )
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "/home/victi/ReactivePlusPlus/build/bin/libsnitch.a")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xDevelopmentx" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}/usr/local/lib/cmake/snitch/snitch-targets.cmake")
    file(DIFFERENT EXPORT_FILE_CHANGED FILES
         "$ENV{DESTDIR}/usr/local/lib/cmake/snitch/snitch-targets.cmake"
         "/home/victi/ReactivePlusPlus/build/_deps/snitch-build/CMakeFiles/Export/_usr/local/lib/cmake/snitch/snitch-targets.cmake")
    if(EXPORT_FILE_CHANGED)
      file(GLOB OLD_CONFIG_FILES "$ENV{DESTDIR}/usr/local/lib/cmake/snitch/snitch-targets-*.cmake")
      if(OLD_CONFIG_FILES)
        message(STATUS "Old export file \"$ENV{DESTDIR}/usr/local/lib/cmake/snitch/snitch-targets.cmake\" will be replaced.  Removing files [${OLD_CONFIG_FILES}].")
        file(REMOVE ${OLD_CONFIG_FILES})
      endif()
    endif()
  endif()
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/usr/local/lib/cmake/snitch/snitch-targets.cmake")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  file(INSTALL DESTINATION "/usr/local/lib/cmake/snitch" TYPE FILE FILES "/home/victi/ReactivePlusPlus/build/_deps/snitch-build/CMakeFiles/Export/_usr/local/lib/cmake/snitch/snitch-targets.cmake")
  if("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^()$")
    list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
     "/usr/local/lib/cmake/snitch/snitch-targets-noconfig.cmake")
    if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
      message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
    endif()
    if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
      message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
    endif()
    file(INSTALL DESTINATION "/usr/local/lib/cmake/snitch" TYPE FILE FILES "/home/victi/ReactivePlusPlus/build/_deps/snitch-build/CMakeFiles/Export/_usr/local/lib/cmake/snitch/snitch-targets-noconfig.cmake")
  endif()
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xDevelopmentx" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/usr/local/lib/cmake/snitch/snitch-config.cmake")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  file(INSTALL DESTINATION "/usr/local/lib/cmake/snitch" TYPE FILE FILES "/home/victi/ReactivePlusPlus/build/_deps/snitch-build/snitch-config.cmake")
endif()

