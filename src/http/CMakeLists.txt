SET(SOURCE_FILE
        main.cpp
        histroy_resource.h
        histroy_resource.cpp
        )

MESSAGE(STATUS "configure Server for http")

SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "/usr/local/share/cmake/Modules/")

FIND_PACKAGE(LibHttpServer REQUIRED)

INCLUDE_DIRECTORIES(${LIBHTTPSERVER_INCLUDE_DIRS})

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
ADD_EXECUTABLE(http_server ${SOURCE_FILE})
TARGET_LINK_LIBRARIES(http_server pthread glog ${LIBHTTPSERVER_LIBRARIES})
