SET(SOURCE_FILE
        main.cpp
        )

MESSAGE(STATUS "configure Server for history")

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
ADD_EXECUTABLE(history_server ${SOURCE_FILE})
TARGET_LINK_LIBRARIES(history_server asio_http glog)
