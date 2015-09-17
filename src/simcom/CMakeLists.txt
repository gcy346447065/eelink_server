SET(SOURCE_FILE
    ../common/log.c
    ../common/log.h
    ../common/yunba_push.c
    ../common/yunba_push.h
    ../common/curl.c
    ../common/curl.h
    ../common/object.c
    ../common/object.h
    ../common/db.c
    ../common/db.h
    ../common/conf.c
    ../common/conf.h

    ../3rd/cJSON/cJSON.c
    ../3rd/cJSON/cJSON.h


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
add_executable(simcom_server ${SOURCE_FILE})
TARGET_LINK_LIBRARIES(simcom_server m event zlog curl mosquitto paho-mqtt3c mysqlclient)
