# if(CMAKE_HOST_SYSTEM_NAME STREQUAL "Windows")
#   include(cmake/open-cpp-coverage.cmake OPTIONAL)
# endif()

# include(cmake/lint-targets.cmake)
# include(cmake/spell-targets.cmake)


if(POLICY CMP0091)
    cmake_policy(SET CMP0091 NEW)
endif()

if(POLICY CMP0144)
    cmake_policy(SET CMP0144 NEW)
endif()
