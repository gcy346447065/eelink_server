SET(SOURCE_FILE
    session_manager.h
    session_manager.c

    jiguang_push.c
    jiguang_push.h

    objectID_leancloud.c
    objectID_leancloud.h

    adler32.c
    adler32.h

    log.c
    log.h

    yunba_push.c
    yunba_push.h

    curl.c
    curl.h

    object.c
    object.h

    db.c
    db.h

    conf.c
    conf.h

    session.h
    session.c

    mqtt.h
    mqtt.c

    cgi2gps.h
    cgi2gps.c

    env.h
    env.c

    sync.h
    sync.c

    msg_app.h
    msg_app.c

    timer.h
    timer.c

    ../3rd/cJSON/cJSON.c
    ../3rd/cJSON/cJSON.h

    call_push.h
    call_push.c

    cencode.h
    cencode.c

    md5.c
    md5.h
    )

MESSAGE(STATUS "configure common library")
link_directories(/usr/lib64/mysql)
set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
add_library(electromble SHARED ${SOURCE_FILE})
TARGET_LINK_LIBRARIES(electromble m event zlog curl mosquitto paho-mqtt3c mysqlclient)
