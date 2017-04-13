SET(SOURCE_FILE
    main.c

    itinerary_object.c
    itinerary_object.h
)

MESSAGE(STATUS "configure Server for itinerary")
link_directories(/usr/lib64/mysql)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
add_executable(itinerary_server ${SOURCE_FILE})
TARGET_LINK_LIBRARIES(itinerary_server electromble)
