add_subdirectory(basic)
add_subdirectory(doxygen)
add_subdirectory(long_chain)
add_subdirectory(two_async_streams)
add_subdirectory(linesfrombytes)

if (RPP_BUILD_SFML_CODE)
    add_subdirectory(sfml)
endif()
