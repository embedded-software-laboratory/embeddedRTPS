cmake_minimum_required(VERSION 3.8)
project(lwip)

#set (CMAKE_EXPORT_COMPILE_COMMANDS ON)
add_subdirectory(Micro-CDR)

SET(LWIP_CONTRIB_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../contrib")
SET(LWIP_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../lwip")

if(WIN32)
	SET(WPDPACK_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../WpdPack/Include")
	SET(PORT_ROOT "${LWIP_CONTRIB_DIR}/ports/win32")
	SET(ARCH_DIR "${PORT_ROOT}/include")
endif (WIN32)

if(UNIX)
	SET(PORT_ROOT "${LWIP_CONTRIB_DIR}/ports/unix")
	SET(ARCH_DIR "${PORT_ROOT}/port/include")
endif (UNIX)

include(${LWIP_CONTRIB_DIR}/ports/CMakeCommon.cmake)

set (LWIP_DEFINITIONS -DLWIP_DEBUG)
set (LWIP_INCLUDE_DIRS
        "${LWIP_DIR}/src/include"
        "${LWIP_CONTRIB_DIR}"
        "${ARCH_DIR}"
        "${CMAKE_CURRENT_SOURCE_DIR}/lwip"
        "${WPDPACK_INCLUDE_DIR}"
        )

include(${LWIP_DIR}/src/Filelists.cmake)
include(${LWIP_CONTRIB_DIR}/Filelists.cmake)
include(${PORT_ROOT}/Filelists.cmake)

if(WIN32)
	add_library(lwip STATIC ${lwipcore_SRCS} ${lwipcore3_SRCS} "${CMAKE_CURRENT_SOURCE_DIR}/lwip/default_netif.c" ${lwipcontribportwindows_SRCS})
endif(WIN32)

if(UNIX)
	add_library(lwip STATIC ${lwipcore_SRCS} ${lwipcore3_SRCS} ${lwipcontribportunix_SRCS})
endif (UNIX)
	
target_include_directories(lwip PUBLIC ${LWIP_INCLUDE_DIRS})
target_compile_options(lwip PRIVATE ${LWIP_COMPILER_FLAGS})
target_compile_definitions(lwip PRIVATE ${LWIP_DEFINITIONS})

if(WIN32)
	target_link_libraries(lwip ${LWIP_SANITIZER_LIBS}  lwipcontribaddons lwipallapps lwipcore lwipcontribportwindows)
endif(WIN32)

if(UNIX)
	target_link_libraries(lwip ${LWIP_SANITIZER_LIBS}  lwipcontribaddons lwipallapps lwipcore lwipcontribportunix)
endif(UNIX)
