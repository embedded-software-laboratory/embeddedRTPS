cmake_minimum_required(VERSION 3.12)

include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})
include_directories(${rtps_SOURCE_DIR}/include ${rtps_SOURCE_DIR})

add_executable(runTests
                        unittests/typesTest.cpp
                        unittests/ThreadPoolTest.cpp
                        unittests/communication/UdpConnectionTest.cpp
                        unittests/storages/PBufWrapperTest.cpp
                        unittests/entities/ParticipantTest.cpp
                        unittests/entities/StatelessReaderTest.cpp
                        unittests/entities/StatelessWriterTest.cpp
                        unittests/messages/MessageFactoryTest.cpp
                        unittests/messages/MessageReceiverTest.cpp
                        unittests/storages/HistoryCacheTest.cpp
                        unittests/utils/udpUtilsTest.cpp
        )
target_link_libraries(runTests gtest gtest_main gmock gmock_main)
target_link_libraries(runTests rtps lwip)
target_include_directories(runTests PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/mocking")

add_executable(runThreadPoolDemo ThreadPoolDemo.cpp)
target_link_libraries(runThreadPoolDemo rtps lwip)

add_executable(lwipTests test_lwip.cpp)
target_link_libraries(lwipTests rtps lwip gtest gtest_main gmock gmock_main)