add_executable(two_async_streams
    two_async_streams.cpp
)

target_link_libraries(two_async_streams PRIVATE RPP::rpp)
set_target_properties(two_async_streams PROPERTIES FOLDER Examples/rpp)