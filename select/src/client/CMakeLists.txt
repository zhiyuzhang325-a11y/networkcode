cmake_minimum_required(VERSION 3.15)
project(client)

set(CMAKE_CXX_STANDARD 17)

add_library(client_lib STATIC clientserver.cpp)

target_include_directories(client_lib PULIC ${PROJECT_SOURCE_DIR})