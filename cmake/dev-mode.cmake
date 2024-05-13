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

if(NOT DEFINED CMAKE_PROJECT_TOP_LEVEL_INCLUDES AND NOT DEFINED CMAKE_TOOLCHAIN_FILE AND EXISTS "${CMAKE_BINARY_DIR}/conan_toolchain.cmake")
    message("Using conan toolchain file: ${CMAKE_BINARY_DIR}/conan_toolchain.cmake")
    include(${CMAKE_BINARY_DIR}/conan_toolchain.cmake)
    set(CMAKE_TOOLCHAIN_FILE ${CMAKE_BINARY_DIR}/conan_toolchain.cmake)
endif()
