cmake_minimum_required(VERSION 3.7)
set(CMAKE_CXX_STANDARD 11)
set (CMAKE_CXX_FLAGS "-Wall -Wextra -Wnon-virtual-dtor -Wpedantic -fno-non-call-exceptions -fno-exceptions -fno-rtti -m32")
# TODO -Wshadow

project(rtps)

add_subdirectory(thirdparty)
target_include_directories(lwip PUBLIC include)

include_directories(include ${LWIP_DIR/src/include} ${microcdr_SOURCE_DIR}/include)

add_library(rtps "")
include(src/CMakeLists.txt)
target_include_directories(rtps PUBLIC include)
target_link_libraries(rtps PRIVATE lwip microcdr)

add_executable(demo main.cpp)
target_link_libraries(demo lwip rtps)

add_subdirectory(test)





