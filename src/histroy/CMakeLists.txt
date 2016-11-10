SET(SOURCE_FILE
    main.cpp
    msg_proc_http.cpp
    msg_proc_http.hpp
        )

MESSAGE(STATUS "configure Server for history")
link_directories(/usr/lib64/mysql)
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
ADD_EXECUTABLE(history_server ${SOURCE_FILE})
TARGET_LINK_LIBRARIES(history_server electromble asio_http glog)
