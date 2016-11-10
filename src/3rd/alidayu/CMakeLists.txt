SET(SOURCE_FILE
        curlResponse.h
        curlResponse.c

        globalConfig.c

        json.h
        json.c

        Md5Util.h
        Md5Util.c

        SingleMap.h
        SingleMap.c

        TaobaoClient.c

        TopRequest.c

        topsdk.h

        TopResponse.h
        TopResponse.c
        WebUtils.h

        WebUtils.c
    )

MESSAGE(STATUS "configure alidayu library")
set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
add_library(alidayu SHARED ${SOURCE_FILE})
TARGET_LINK_LIBRARIES(alidayu m curl)
