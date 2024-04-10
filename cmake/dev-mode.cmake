# if(CMAKE_HOST_SYSTEM_NAME STREQUAL "Windows")
#   include(cmake/open-cpp-coverage.cmake OPTIONAL)
# endif()

# include(cmake/lint-targets.cmake)
# include(cmake/spell-targets.cmake)

if(CMAKE_EXPORT_COMPILE_COMMANDS)
    set(CMAKE_CXX_STANDARD_INCLUDE_DIRECTORIES ${CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES})
endif()
