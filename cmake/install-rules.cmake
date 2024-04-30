include(CMakePackageConfigHelpers)
include(GNUInstallDirs)

# find_package(<package>) call for consumers to find this project
set(package RPP)

install(
    DIRECTORY
       src/rpp
       src/extensions/rppqt
    DESTINATION
        "${CMAKE_INSTALL_INCLUDEDIR}"
    COMPONENT
        RPP_Development
)

install(
    TARGETS rpp
    EXPORT RPPTargets
    INCLUDES DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/rpp"
)

install(
    TARGETS rppqt
    EXPORT RPPTargets
    INCLUDES DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/rppqt"
)

write_basic_package_version_file(
    "${package}ConfigVersion.cmake"
    COMPATIBILITY SameMajorVersion
    ARCH_INDEPENDENT
)

# Allow package maintainers to freely override the path for the configs
set(RPP_INSTALL_CMAKEDIR "${CMAKE_INSTALL_DATADIR}/${package}" CACHE PATH "CMake package config location relative to the install prefix")
mark_as_advanced(RPP_INSTALL_CMAKEDIR)

configure_package_config_file(cmake/install-config.cmake.in "${package}Config.cmake"
  INSTALL_DESTINATION "${RPP_INSTALL_CMAKEDIR}"
)

install(
    FILES
        "${PROJECT_BINARY_DIR}/${package}Config.cmake"
        "${PROJECT_BINARY_DIR}/${package}ConfigVersion.cmake"
    DESTINATION
        "${RPP_INSTALL_CMAKEDIR}"
    COMPONENT
        RPP_Development
)

install(
    EXPORT RPPTargets
    NAMESPACE RPP::
    DESTINATION "${RPP_INSTALL_CMAKEDIR}"
    COMPONENT RPP_Development
)
