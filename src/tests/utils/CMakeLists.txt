add_library(rpp_tests_utils INTERFACE)
target_include_directories(rpp_tests_utils INTERFACE .)
target_link_libraries(rpp_tests_utils INTERFACE rpp)


add_library(rpp_doctest_main doctest_main.cpp)
target_link_libraries(rpp_doctest_main PUBLIC doctest::doctest)