cmake_minimum_required(VERSION 3.8)
project(lwip)

#set (CMAKE_EXPORT_COMPILE_COMMANDS ON)

SET(LWIP_CONTRIB_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../contrib")
SET(LWIP_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../lwip")
SET(WPDPACK_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../WpdPack")
set(ARCH_DIR "${LWIP_CONTRIB_DIR}/ports/win32/include")

include(${LWIP_CONTRIB_DIR}/ports/CMakeCommon.cmake)

set (LWIP_DEFINITIONS -DLWIP_DEBUG)
set (LWIP_INCLUDE_DIRS
        "${LWIP_DIR}/src/include"
        "${LWIP_CONTRIB_DIR}"
        "${ARCH_DIR}"
        "${CMAKE_CURRENT_SOURCE_DIR}/lwip"
        "${WPDPACK_DIR}/Include"
        )

include(${LWIP_DIR}/src/Filelists.cmake)
include(${LWIP_CONTRIB_DIR}/Filelists.cmake)
include(${LWIP_CONTRIB_DIR}/ports/win32/Filelists.cmake)

add_library(lwip STATIC ${lwipcore_SRCS} ${lwipcore3_SRCS} ${lwipcontribportwindows_SRCS} "${CMAKE_CURRENT_SOURCE_DIR}/lwip/default_netif.c")
target_include_directories(lwip PUBLIC ${LWIP_INCLUDE_DIRS})
target_compile_options(lwip PRIVATE ${LWIP_COMPILER_FLAGS})
target_compile_definitions(lwip PRIVATE ${LWIP_DEFINITIONS})
target_link_libraries(lwip ${LWIP_SANITIZER_LIBS}  lwipcontribaddons lwipallapps lwipcore lwipcontribportwindows)



add_subdirectory(googletest)