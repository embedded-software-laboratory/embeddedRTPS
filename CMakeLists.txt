cmake_minimum_required(VERSION 3.7)
set(CMAKE_CXX_STANDARD 11)
set (CMAKE_CXX_FLAGS "-Wall -fno-exceptions -fno-non-call-exceptions -fno-rtti -m32")

project(rtps)

add_subdirectory(thirdparty)
target_include_directories(lwip PUBLIC include)

add_library(rtps "")
include(src/CMakeLists.txt)
target_include_directories(rtps PUBLIC include)
target_link_libraries(rtps PRIVATE lwip)

add_executable(demo main.cpp)
target_link_libraries(demo lwip)

add_subdirectory(test)





