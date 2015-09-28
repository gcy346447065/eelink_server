cmake_minimum_required(VERSION 2.8)
project(Electrombile)

#SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu99")
SET(CMAKE_C_FLAGS "-std=c99 -Wall -Wextra -Wshadow -Wcast-qual")
ADD_SUBDIRECTORY(src)
SET(CMAKE_INSTALL_PREFIX /usr/local)