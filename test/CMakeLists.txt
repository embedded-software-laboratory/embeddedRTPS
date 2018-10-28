cmake_minimum_required(VERSION 3.12)

include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})
include_directories(${rtps_SOURCE_DIR}/include ${rtps_SOURCE_DIR})

add_executable(runTests test_udpConnection.cpp
                        test_threadPool.cpp
                        test_lwip.cpp
        )
target_link_libraries(runTests gtest gtest_main gmock gmock_main)
target_link_libraries(runTests rtps lwip)
target_include_directories(runTests PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/mocking")

add_executable(runThreadPoolTest test_threadPool.cpp)
target_link_libraries(runThreadPoolTest rtps lwip)
