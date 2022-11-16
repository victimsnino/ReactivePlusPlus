add_subdirectory(basic)
add_subdirectory(doxygen)
add_subdirectory(petri)

if (RPP_BUILD_SFML_CODE)
    add_subdirectory(sfml)
endif()