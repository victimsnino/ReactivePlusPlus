cmake_minimum_required(VERSION 3.12)
project(test_package_qt LANGUAGES CXX)

find_package(RPP REQUIRED CONFIG)

set(RPP_BUILD_QT_CODE ON)
set(RPP_BUILD_EXAMPLES ON)
include(../../../../cmake/dependencies.cmake)

add_executable(${PROJECT_NAME} ../interactive_window/interactive_window.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE RPP::rppqt)
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)

rpp_add_qt_support_to_executable(${PROJECT_NAME})
