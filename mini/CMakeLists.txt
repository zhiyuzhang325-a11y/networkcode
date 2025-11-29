cmake_minimum_required(VERSION 3.15)
project(test)

set(CMAKE_CXX_STANDARD 17)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin)

set(SERVER_SOURCES ${PROJECT_SOURCE_DIR}/src/server.cpp)
set(CLIENT_SOURCES ${PROJECT_SOURCE_DIR}/src/client.cpp)

add_executable(server ${SERVER_SOURCES})
add_executable(client ${CLIENT_SOURCES})