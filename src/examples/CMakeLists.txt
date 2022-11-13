add_subdirectory(doxygen)
add_subdirectory(rpp)

if (RPP_BUILD_SFML_CODE)
    add_subdirectory(sfml)
endif()

if(RPP_BUILD_QT_CODE)
    add_subdirectory(rppqt)
endif()