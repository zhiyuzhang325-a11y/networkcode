cmake_minimum_required(VERSION 3.15)
project(test)

set(CMAKE_CXX_STANDARD 17)

# include 目录
include_directories(${PROJECT_SOURCE_DIR}/include)

# 可执行文件输出到 bin/
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
# 静态库输出到 lib/
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

set(SRC ${PROJECT_SOURCE_DIR}/src)

file(GLOB_RECURSE CLIENT_SRC ${SRC}/client/*.cpp ${SRC}/main/client.cpp ${SRC}/utility/*.cpp)
file(GLOB_RECURSE SERVER_SRC ${SRC}/server/*.cpp ${SRC}/main/server.cpp ${SRC}/utility/*.cpp)

add_executable(client_app ${CLIENT_SRC})
add_executable(server_app ${SERVER_SRC})

# add_subdirectory(client)
# add_subdirectory(server)
# add_subdirectory(src/main)
