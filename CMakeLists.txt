cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 11)

set (CMAKE_CXX_FLAGS "-O3 -Wall -Wextra -Wnon-virtual-dtor -Wpedantic -fno-non-call-exceptions -fno-exceptions -fno-rtti")
# TODO -Wshadow

if(WIN32)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m32")
endif(WIN32)

project(rtps)

add_subdirectory(thirdparty)
target_include_directories(lwip PUBLIC include)

include_directories(include ${LWIP_DIR/src/include} ${microcdr_SOURCE_DIR}/include)

add_library(rtps "")
include(src/CMakeLists.txt)
target_include_directories(rtps PUBLIC include)
target_link_libraries(rtps PRIVATE lwip microcdr)

add_subdirectory(test)
