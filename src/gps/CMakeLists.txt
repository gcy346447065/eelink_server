SET(SOURCE_FILE
    main.cpp

    UdpServerDataHandle.cpp
    UdpServerDataHandle.h

    DB.cpp
    DB.h

    mqtt_top.cpp
    mqtt_top.h

    Message.cpp
    Message.h

    logger.cpp
    logger.h

    ../3rd/MySQLConnectorWrapper/mysqlconn_wrapper.cpp
    ../3rd/MySQLConnectorWrapper/mysqlconn_wrapper.h protocol.h
)

MESSAGE(STATUS "configure Server for gps")
link_directories(/usr/lib64/mysql)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
add_executable(gps_server ${SOURCE_FILE})
TARGET_LINK_LIBRARIES(gps_server server ev pthread log4cpp mysqlcppconn mosquittopp electromble)
