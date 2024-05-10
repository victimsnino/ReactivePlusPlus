find_package(Threads REQUIRED)

macro(rpp_handle_3rdparty TARGET_NAME)
  get_target_property(TARGET_TYPE ${TARGET_NAME} TYPE)
  if (${TARGET_TYPE} STREQUAL "INTERFACE_LIBRARY")
    target_compile_options(${TARGET_NAME} INTERFACE "-w")
  else()
    target_compile_options(${TARGET_NAME} PRIVATE "-w")
  endif()

  set_target_properties(${TARGET_NAME} PROPERTIES INTERFACE_SYSTEM_INCLUDE_DIRECTORIES $<TARGET_PROPERTY:${TARGET_NAME},INTERFACE_INCLUDE_DIRECTORIES>)
  set_target_properties(${TARGET_NAME} PROPERTIES CXX_CLANG_TIDY "")
  set_target_properties(${TARGET_NAME} PROPERTIES CXX_CPPCHECK "")
  set_target_properties(${TARGET_NAME} PROPERTIES FOLDER 3rdparty)
endmacro()

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

# ========================== GRPC ====================================
if (RPP_BUILD_GRPC_CODE AND (RPP_BUILD_TESTS OR RPP_BUILD_EXAMPLES))
  find_package(Protobuf CONFIG REQUIRED)
  find_package(gRPC CONFIG REQUIRED)
  find_package(absl CONFIG REQUIRED)

  rpp_handle_3rdparty(gRPC::grpc++)
  rpp_handle_3rdparty(protobuf::protobuf)

  macro(rpp_add_proto_files TARGET FILES)
    add_library(${TARGET}_proto STATIC ${FILES})

    target_link_libraries(${TARGET}_proto
        PUBLIC
            gRPC::grpc++
            protobuf::libprotobuf
            ${grpc_LIBRARIES_TARGETS}
    )

    target_include_directories(${TARGET}_proto PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

    get_target_property(grpc_cpp_plugin_location gRPC::grpc_cpp_plugin LOCATION )
    protobuf_generate(TARGET ${TARGET}_proto OUT_VAR PROTO_FILES LANGUAGE cpp )

    protobuf_generate(
        TARGET ${TARGET}_proto
        LANGUAGE grpc
        OUT_VAR GRPC_PROTO_FILES
        # PROTOC_OUT_DIR "${PROTO_BINARY_DIR}"
        PLUGIN protoc-gen-grpc=${grpc_cpp_plugin_location}
        GENERATE_EXTENSIONS .grpc.pb.h .grpc.pb.cc)

    target_link_libraries(${TARGET} PRIVATE ${TARGET}_proto)

    set_target_properties(${TARGET}_proto PROPERTIES INTERFACE_SYSTEM_INCLUDE_DIRECTORIES $<TARGET_PROPERTY:${TARGET}_proto,INTERFACE_INCLUDE_DIRECTORIES>)
    set_target_properties(${TARGET}_proto PROPERTIES CXX_CLANG_TIDY "")
    set_target_properties(${TARGET}_proto PROPERTIES CXX_CPPCHECK "")
  endmacro()
endif()

macro(rpp_fetch_library_extended NAME URL TAG TARGET_NAME)
  Include(FetchContent)
  set(BUILD_SHARED_LIBS OFF CACHE INTERNAL "Build SHARED libraries")

  FetchContent_Declare(
    ${NAME}
    GIT_REPOSITORY ${URL}
    GIT_TAG        ${TAG}
    GIT_SHALLOW TRUE
  )

  FetchContent_MakeAvailable(${NAME})
  rpp_handle_3rdparty(${TARGET_NAME})
endmacro()

macro(rpp_fetch_library NAME URL TAG)
  rpp_fetch_library_extended(${NAME} ${URL} ${TAG} ${NAME})
endmacro()

# ==================== RXCPP =======================
if (RPP_BUILD_RXCPP AND RPP_BUILD_BENCHMARKS)
  set(RXCPP_DISABLE_TESTS_AND_EXAMPLES 1)

  rpp_fetch_library(rxcpp https://github.com/ReactiveX/RxCpp.git origin/main)
endif()

# ===================== Snitch ===================
if (RPP_BUILD_TESTS)
  SET(SNITCH_CONSTEXPR_FLOAT_USE_BITCAST 0)
  rpp_fetch_library(snitch https://github.com/cschreib/snitch.git v1.2.4)
endif()


# ==================== Nanobench =================
if (RPP_BUILD_BENCHMARKS)
  rpp_fetch_library(nanobench https://github.com/martinus/nanobench.git master)
endif()
