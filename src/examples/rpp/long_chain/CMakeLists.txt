add_executable(long_chain_sample
    long_chain.cpp
)

target_link_libraries(long_chain_sample PRIVATE RPP::rpp)
set_target_properties(long_chain_sample PROPERTIES FOLDER Examples/rpp)