
####### Expanded from @PACKAGE_INIT@ by configure_package_config_file() #######
####### Any changes to this file will be overwritten by the next CMake run ####
####### The input file was snitch-config.cmake.in                            ########

get_filename_component(PACKAGE_PREFIX_DIR "${CMAKE_CURRENT_LIST_DIR}/../" ABSOLUTE)

####################################################################################

file(GLOB CONFIG_FILES "${CMAKE_CURRENT_LIST_DIR}/snitch*-targets.cmake")
foreach(f ${CONFIG_FILES})
    include(${f})

    string(REGEX MATCH "${CMAKE_CURRENT_LIST_DIR}/(snitch.*)-targets.cmake" match ${f})
    set(target ${CMAKE_MATCH_1})

    if (NOT TARGET snitch::${target})
        add_library(snitch::${target} ALIAS ${target})
    endif()
endforeach()
