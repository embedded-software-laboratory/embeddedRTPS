cmake_minimum_required(VERSION 3.12)

include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})
include_directories(${rtps_SOURCE_DIR}/include ${rtps_SOURCE_DIR})

add_executable(runTests test1.cpp)
target_link_libraries(runTests gtest gtest_main)
target_link_libraries(runTests rtps)


