cmake_minimum_required(VERSION 3.7)

target_sources(rtps
        PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/communication/PBufWrapper.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/communication/UdpDriver.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/entities/StatelessWriter.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/rtps.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/ThreadPool.cpp"
)


