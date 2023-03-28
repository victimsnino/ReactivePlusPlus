#                  ReactivePlusPlus library
#
#          Copyright Aleksey Loginov 2022 - present.
# Distributed under the Boost Software License, Version 1.0.
#    (See accompanying file LICENSE_1_0.txt or copy at
#          https://www.boost.org/LICENSE_1_0.txt)
#
# Project home: https://github.com/victimsnino/ReactivePlusPlus
#

add_subdirectory(utils)

macro(rpp_register_tests module)
  file(GLOB_RECURSE RPP_FILES "${module}/test_*.cpp")
  foreach(SOURCE ${RPP_FILES})
    get_filename_component(BASE_NAME ${SOURCE} NAME_WE)

    set(TARGET ${BASE_NAME})

    add_executable(${TARGET} ${SOURCE})
    target_link_libraries(${TARGET} PRIVATE snitch::snitch rpp_tests_utils ${module})
    set_target_properties(${TARGET} PROPERTIES FOLDER Tests/Suites/${module})

    add_test(NAME ${TARGET} COMMAND $<TARGET_FILE:${TARGET}>)

    if (${module} STREQUAL rppqt)
      rpp_add_qt_support_to_executable(${TARGET})
    endif()

    if(MSVC)
      target_compile_options(${TARGET} PRIVATE /W4 /WX)
    else()
      target_compile_options(${TARGET} PRIVATE -Wall -Wextra -Wpedantic -Werror -Wconversion)
    endif()

  endforeach()
endmacro()

rpp_register_tests(rpp)

# if (RPP_BUILD_QT_CODE)
#   rpp_register_tests(rppqt)
# endif()