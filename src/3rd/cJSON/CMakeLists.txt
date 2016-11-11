SET(SOURCE_FILE
        cJSON.h
        cJSON.c
    )

MESSAGE(STATUS "configure JSON library")
set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
add_library(json STATIC ${SOURCE_FILE})
