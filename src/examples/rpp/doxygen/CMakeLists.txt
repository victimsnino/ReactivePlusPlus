file(GLOB_RECURSE FILES "*.cpp")

foreach(SOURCE ${FILES})
    get_filename_component(BASE_NAME ${SOURCE} NAME_WE)
    set(TARGET ${BASE_NAME}_doxygen_sample)
    add_executable(${TARGET} ${SOURCE})
    target_link_libraries(${TARGET} PRIVATE rpp)
    set_target_properties(${TARGET} PROPERTIES FOLDER Examples/rpp/Doxygen)
endforeach()
