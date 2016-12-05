SET(SOURCE_FILE
        main.c

        msg_http.c
        msg_http.h

        msg_proc_http.c
        msg_proc_http.h

        gps_http.c
        gps_http.h

        device_http.c
        device_http.h

        itinerary_http.c
        itinerary_http.h

        telephone_http.c
        telephone_http.h

        package_http.c
        package_http.h


        session_http.c
        session_http.h
        )

MESSAGE(STATUS "configure Server for http")

SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "/usr/local/share/cmake/Modules/")
link_directories(/usr/lib64/mysql)
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
ADD_EXECUTABLE(http_server ${SOURCE_FILE})
TARGET_LINK_LIBRARIES(http_server electromble glog)
