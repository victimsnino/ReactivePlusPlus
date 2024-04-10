add_library(rpp_tests_utils INTERFACE)
target_include_directories(rpp_tests_utils INTERFACE .)
target_link_libraries(rpp_tests_utils INTERFACE rpp)
