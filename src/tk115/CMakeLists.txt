SET(SOURCE_FILE
    main.c
    mqtt.c
    mqtt.h
    session.c
    session.h
    server_tk115.c
    server_tk115.h
    msg_sch_tk115.c
    msg_sch_tk115.h
    msg_proc_app.c
    msg_proc_app.h
    msg_proc_tk115.c
    msg_proc_tk115.h
    msg_tk115.c
    msg_tk115.h)

MESSAGE(STATUS "configure Server for TK115")
link_directories(/usr/lib64/mysql)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
add_executable(tk115_server ${SOURCE_FILE})
TARGET_LINK_LIBRARIES(tk115_server m event zlog curl mosquitto paho-mqtt3c mysqlclient electromble)
