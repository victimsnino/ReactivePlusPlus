set(TARGET rppgrpc_communication)

rpp_add_proto_target(${TARGET}_proto protocol.proto)

add_executable(${TARGET}_server server.cpp)
target_link_libraries(${TARGET}_server PRIVATE ${TARGET}_proto rppgrpc)

add_executable(${TARGET}_client client.cpp)
target_link_libraries(${TARGET}_client PRIVATE ${TARGET}_proto rppgrpc)
