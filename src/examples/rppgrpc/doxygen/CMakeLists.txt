file(GLOB_RECURSE FILES "*.cpp")

rpp_add_proto_target(doyxgen_grpc_proto protocol.proto)

foreach(SOURCE ${FILES})
    get_filename_component(BASE_NAME ${SOURCE} NAME_WE)
    set(TARGET ${BASE_NAME}_grpc_doxygen_sample)
    add_executable(${TARGET} ${SOURCE})
    target_link_libraries(${TARGET} PRIVATE rpp rppgrpc doyxgen_grpc_proto)
    set_target_properties(${TARGET} PROPERTIES FOLDER Examples/rppqt/Doxygen)
endforeach()
