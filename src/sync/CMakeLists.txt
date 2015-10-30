SET(SOURCE_FILE
    main.c

    server_sync.h
    server_sync.c

    leancloud_req.h
    leancloud_req.c

    leancloud_rsp.h
    leancloud_rsp.c

    msg_proc.h
    msg_proc.c)

MESSAGE(STATUS "configure Server for sync")
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
add_executable(sync_server ${SOURCE_FILE})
TARGET_LINK_LIBRARIES(sync_server electromble)
