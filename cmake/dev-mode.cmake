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

# conan install . --output-folder=build --build=missing -s compiler.cppstd=20 -c tools.system.package_manager:mode=install -c tools.system.package_manager:sudo=True
if(NOT DEFINED CMAKE_TOOLCHAIN_FILE AND EXISTS "${CMAKE_BINARY_DIR}/conan_toolchain.cmake")
    include(${CMAKE_BINARY_DIR}/conan_toolchain.cmake)
    set(CMAKE_TOOLCHAIN_FILE ${CMAKE_BINARY_DIR}/conan_toolchain.cmake)
endif()
