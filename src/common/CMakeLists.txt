SET(SOURCE_FILE
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

    ../3rd/cJSON/cJSON.c
    ../3rd/cJSON/cJSON.h
    )

MESSAGE(STATUS "configure common library")
link_directories(/usr/lib64/mysql)
set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
add_library(electromble SHARED ${SOURCE_FILE})
TARGET_LINK_LIBRARIES(electromble m event zlog curl mosquitto paho-mqtt3c mysqlclient)
