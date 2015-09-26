SET(SOURCE_FILE
    main.c
    mqtt.c
    mqtt.h
    server_simcom.c
    server_simcom.h
    msg_proc_app.c
    msg_proc_app.h
    msg_proc_simcom.c
    msg_proc_simcom.h
    msg_simcom.c
    msg_simcom.h
    env.c
    env.h
    session.c
    session.h)

MESSAGE(STATUS "configure Server for SIMCOM")
link_directories(/usr/lib64/mysql)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
add_executable(simcom_server ${SOURCE_FILE})
TARGET_LINK_LIBRARIES(simcom_server electromble)
