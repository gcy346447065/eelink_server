SET(SOURCE_FILE
    jiguang_push.c
    jiguang_push.h

    adler32.c
    adler32.h

    log.c
    log.h

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

    env.h
    env.c

    sync.h
    sync.c

    msg_http.h
    msg_http.c

    request_table.h
    request_table.c

    msg_app.h
    msg_app.c

    timer.h
    timer.c

    phone_alarm.h
    phone_alarm.c
)

MESSAGE(STATUS "configure common library")
link_directories(/usr/lib64/mysql)
set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
add_library(electromble SHARED ${SOURCE_FILE})
TARGET_LINK_LIBRARIES(electromble alidayu m event zlog curl mosquitto paho-mqtt3c mysqlclient json)
