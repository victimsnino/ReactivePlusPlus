add_executable(basic_sample
    basic.cpp
)

target_link_libraries(basic_sample PRIVATE RPP::rpp)
set_target_properties(basic_sample PROPERTIES FOLDER Examples)
