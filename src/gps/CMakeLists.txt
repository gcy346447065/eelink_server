SET(SOURCE_FILE
    main.cpp
        UdpServerDataHandle.cpp
        UdpServerDataHandle.h
        DB.cpp
        DB.h
        Message.cpp
        Message.h)

MESSAGE(STATUS "configure Server for gps")
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
add_executable(gps_server ${SOURCE_FILE})
TARGET_LINK_LIBRARIES(gps_server server ev pthread glog)
