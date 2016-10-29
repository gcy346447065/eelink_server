SET(SOURCE_FILE
    main.c

    firmware_upgrade.c
    firmware_upgrade.h

    msg_manager.c
    msg_manager.h

    msg_proc_app.c
    msg_proc_app.h

    msg_proc_manager.c
    msg_proc_manager.h

    msg_proc_simcom.c
    msg_proc_simcom.h

    msg_simcom.c
    msg_simcom.h

    server_manager.c
    server_manager.h

    server_simcom.c
    server_simcom.h

    session_manager.h
    session_manager.c

    )

MESSAGE(STATUS "configure Server for SIMCOM")
link_directories(/usr/lib64/mysql)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
add_executable(simcom_server ${SOURCE_FILE})
TARGET_LINK_LIBRARIES(simcom_server electromble)
