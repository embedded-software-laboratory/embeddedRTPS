cmake_minimum_required(VERSION 3.7)

target_sources(rtps
        PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/rtps.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/ThreadPool.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/communication/UdpDriver.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/discovery/SPDPAgent.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/discovery/SEDPAgent.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/discovery/ParticipantProxyData.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/discovery/BuiltInTopicData.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/entities/Domain.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/entities/Participant.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/entities/StatefullReader.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/entities/StatefullWriter.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/entities/StatelessReader.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/entities/StatelessWriter.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/messages/MessageReceiver.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/storages/HistoryCache.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/storages/PBufWrapper.cpp"
)


