cmake_minimum_required(VERSION 3.12)

include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})
include_directories(${rtps_SOURCE_DIR}/include ${rtps_SOURCE_DIR})

add_executable(runTests ThreadPoolTest.cpp
                        typesTest.cpp
                        communication/UdpConnectionTest.cpp
                        communication/PBufWrapperTest.cpp
                        entities/StatelessWriterTest.cpp
        )
target_link_libraries(runTests gtest gtest_main gmock gmock_main)
target_link_libraries(runTests rtps lwip)
target_include_directories(runTests PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/mocking")

add_executable(runThreadPoolTest ThreadPoolDemo.cpp)
target_link_libraries(runThreadPoolTest rtps lwip)

add_executable(lwipTests test_lwip.cpp)
target_link_libraries(lwipTests rtps lwip)