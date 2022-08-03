project(asoa_embedded_rtps)
cmake_minimum_required(VERSION 3.15)

if(NOT DEFINED IP_TARGET)
	message(FATAL_ERROR "Please define the target's IP address using -DIP_TARGET.")
endif()

string(REPLACE "." "," IP_IN_CODE ${IP_TARGET})

# copy FreeRTOS configuration to include directory
if (${ARCHITECTURE} STREQUAL "armr5")
	configure_file(${CMAKE_SOURCE_DIR}/targets/cortex-r5/FreeRTOSConfig.h ${DIR_FREERTOS_HEADER} COPYONLY)
	configure_file(${CMAKE_SOURCE_DIR}/targets/cortex-r5/lwipopts.h ${DIR_FREERTOS_HEADER} COPYONLY)
elseif(${ARCHITECTURE} STREQUAL "esp32")
	# configure_file(${CMAKE_SOURCE_DIR}/targets/esp32/FreeRTOSConfig.h ${DIR_FREERTOS_HEADER} COPYONLY)
	# configure_file(${CMAKE_SOURCE_DIR}/targets/esp32/lwipopts.h ${DIR_FREERTOS_HEADER} COPYONLY)
endif()

# TODO distinguish between platforms?
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/include/rtps/config_r5.h.in" "${CMAKE_CURRENT_SOURCE_DIR}/include/rtps/config.h" @ONLY)

set(INSTALL_INCLUDE_DST  "${CMAKE_INSTALL_PREFIX}/include/embeddedRTPS/")

set(SRC	"src/communication/UdpDriver.cpp"
		"src/discovery/ParticipantProxyData.cpp"
		"src/discovery/SEDPAgent.cpp"
		"src/discovery/SPDPAgent.cpp"
		"src/discovery/TopicData.cpp"
		"src/entities/Domain.cpp"
		"src/entities/Participant.cpp"
		"src/entities/StatelessReader.cpp"
		"src/messages/MessageReceiver.cpp"
		"src/messages/MessageTypes.cpp"
		"src/storages/PBufWrapper.cpp"
		"src/rtps.cpp"
		"src/ThreadPool.cpp")
		
add_library(embedded_rtps ${SRC})
target_include_directories(embedded_rtps PUBLIC 
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
	$<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/common/Micro-CDR/include>
	$<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/common/Micro-CDR/include>
	$<BUILD_INTERFACE:${INC_DIR_R5_PLATFORM}>)

if (ASOA_SECURITY)
	target_link_libraries(embedded_rtps PUBLIC asoa_security_middleware)
endif ()