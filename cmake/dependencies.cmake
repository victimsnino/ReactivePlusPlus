find_package(Threads REQUIRED)

# ===================== SFML =======================
if (RPP_BUILD_SFML_CODE AND RPP_BUILD_EXAMPLES)
    find_package(SFML COMPONENTS graphics system window REQUIRED)
endif()

# ==================== QT ==========================
if (RPP_BUILD_QT_CODE AND (RPP_BUILD_TESTS OR RPP_BUILD_EXAMPLES))
  find_package(Qt6 COMPONENTS Widgets QUIET)
  if (Qt6_FOUND)
    SET(RPP_QT_TARGET Qt6)
  else()
    message("-- RPP: Can't find Qt6, searching for Qt5...")
    find_package(Qt5 REQUIRED COMPONENTS Widgets)
    SET(RPP_QT_TARGET Qt5)
  endif()

  message("-- RPP: Found QT version: ${RPP_QT_TARGET}")
  macro(rpp_add_qt_support_to_executable TARGET)
    target_link_libraries(${TARGET} PRIVATE ${RPP_QT_TARGET}::Widgets)
    set_target_properties(${TARGET} PROPERTIES AUTOMOC TRUE)
    if (WIN32)
      add_custom_command (TARGET ${TARGET} POST_BUILD  COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:${RPP_QT_TARGET}::Core> $<TARGET_FILE_DIR:${TARGET}>)
      add_custom_command (TARGET ${TARGET} POST_BUILD  COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:${RPP_QT_TARGET}::Widgets> $<TARGET_FILE_DIR:${TARGET}>)
      add_custom_command (TARGET ${TARGET} POST_BUILD  COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:${RPP_QT_TARGET}::Gui> $<TARGET_FILE_DIR:${TARGET}>)
    endif()
  endmacro()
endif()

# ==================== RXCPP =======================
if (RPP_BUILD_RXCPP AND RPP_BUILD_BENCHMARKS)
  set(RXCPP_DISABLE_TESTS_AND_EXAMPLES 1)

  Include(FetchContent)

  FetchContent_Declare(
    RxCpp
    GIT_REPOSITORY https://github.com/ReactiveX/RxCpp.git
    GIT_TAG        origin/main
    GIT_SHALLOW TRUE
  )

  FetchContent_MakeAvailable(RxCpp)

  set_target_properties(rxcpp PROPERTIES INTERFACE_SYSTEM_INCLUDE_DIRECTORIES $<TARGET_PROPERTY:rxcpp,INTERFACE_INCLUDE_DIRECTORIES>)
  set_target_properties(rxcpp PROPERTIES CXX_CLANG_TIDY "")
  set_target_properties(rxcpp PROPERTIES CXX_CPPCHECK "")
  set_target_properties(rxcpp PROPERTIES FOLDER 3rdparty)

endif()

# ===================== Snitch ===================
if (RPP_BUILD_TESTS)
  Include(FetchContent)

  FetchContent_Declare(snitch
      GIT_REPOSITORY https://github.com/cschreib/snitch.git
      GIT_TAG        main
      GIT_SHALLOW TRUE
    )
  FetchContent_MakeAvailable(snitch)

  target_compile_options(snitch PRIVATE "-w")
  set_target_properties(snitch PROPERTIES INTERFACE_SYSTEM_INCLUDE_DIRECTORIES $<TARGET_PROPERTY:snitch,INTERFACE_INCLUDE_DIRECTORIES>)
  set_target_properties(snitch PROPERTIES CXX_CLANG_TIDY "")
  set_target_properties(snitch PROPERTIES CXX_CPPCHECK "")
  set_target_properties(snitch PROPERTIES FOLDER 3rdparty)

endif()


# ==================== Nanobench =================
if (RPP_BUILD_BENCHMARKS)
  Include(FetchContent)

  FetchContent_Declare(nanobench
      GIT_REPOSITORY https://github.com/martinus/nanobench.git
      GIT_TAG        master
      GIT_SHALLOW    TRUE
    )
  FetchContent_MakeAvailable(nanobench)

  target_compile_options(nanobench PRIVATE "-w")
  set_target_properties(nanobench PROPERTIES INTERFACE_SYSTEM_INCLUDE_DIRECTORIES $<TARGET_PROPERTY:nanobench,INTERFACE_INCLUDE_DIRECTORIES>)
  set_target_properties(nanobench PROPERTIES CXX_CLANG_TIDY "")
  set_target_properties(nanobench PROPERTIES CXX_CPPCHECK "")
  set_target_properties(nanobench PROPERTIES FOLDER 3rdparty)

endif()