#                  ReactivePlusPlus library
#
#          Copyright Aleksey Loginov 2022 - present.
# Distributed under the Boost Software License, Version 1.0.
#    (See accompanying file LICENSE_1_0.txt or copy at
#          https://www.boost.org/LICENSE_1_0.txt)
#
# Project home: https://github.com/victimsnino/ReactivePlusPlus
#

add_subdirectory(rpp)

if(RPP_BUILD_TESTS)
    add_subdirectory(tests)
endif()

# For IDE developers because both CLion and VSCode Intellisense may not support
# interface target well.
add_subdirectory(samples)
