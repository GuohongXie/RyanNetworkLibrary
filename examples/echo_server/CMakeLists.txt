# 设置项目名称
project(echo_server C CXX)
#enable_testing()

# 指定使用 CMake 的最低版本号
cmake_minimum_required(VERSION 3.15)
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")

# 添加搜索路径
include_directories(
    ${PROJECT_SOURCE_DIR}/../../src/base
    ${PROJECT_SOURCE_DIR}/../../src/buffer
    ${PROJECT_SOURCE_DIR}/../../src/logger
    ${PROJECT_SOURCE_DIR}/../../src/memory_pool
    ${PROJECT_SOURCE_DIR}/../../src/net
    ${PROJECT_SOURCE_DIR}/../../src/tcp_connection
    ${PROJECT_SOURCE_DIR}/../../src/timer
    ${Boost_INCLUDE_DIRS}
    )

find_package(Boost REQUIRED COMPONENTS unit_test_framework)
# check if boost was found
if(Boost_FOUND)
    message ("boost found")
else()
    message (FATAL_ERROR "Cannot find Boost")
endif()

# 包含这些目录下的.cc文件
# 下面这两个会把目录及其子目录的所有.cc文件都包括
#aux_source_directory(${PROJECT_SOURCE_DIR}/../../src/logger SRC_LOG)
#aux_source_directory(${PROJECT_SOURCE_DIR}/../../src/logger SRC_LOG)
#如果想要不包含子目录下的文件，则使用file
file(GLOB SRC_BASE ${PROJECT_SOURCE_DIR}/../../src/base/*.cc)
file(GLOB SRC_BUFFER ${PROJECT_SOURCE_DIR}/../../src/buffer/*.cc)
file(GLOB SRC_LOG ${PROJECT_SOURCE_DIR}/../../src/logger/*.cc)
file(GLOB SRC_MEMORY_POOL ${PROJECT_SOURCE_DIR}/../../src/memory_pool/*.cc)
file(GLOB SRC_NET ${PROJECT_SOURCE_DIR}/../../src/net/*.cc)
file(GLOB SRC_TCP_CONNECTION ${PROJECT_SOURCE_DIR}/../../src/tcp_connection/*.cc)
file(GLOB SRC_TIMER ${PROJECT_SOURCE_DIR}/../../src/timer/*.cc)


#set (SRC_channel_test  ${PROJECT_SOURCE_DIR}/channel_test.cc)
set (SRC_echo_server  ${PROJECT_SOURCE_DIR}/echo_server.cc)
set (SRC_echo_server_async  ${PROJECT_SOURCE_DIR}/echo_server_async.cc)

# 设置编译选项
set(CXX_FLAGS
    -g
    -Wall
    -std=c++11
    )


#add an executable with the above sources
#add_executable(channel_test  ${SRC_channel_test} ${SRC_LOG} ${SRC_BUFFER} ${SRC_BASE} ${SRC_NET} ${SRC_TIMER})
#target_link_libraries(channel_test  pthread)

add_executable(echo_server ${SRC_echo_server} ${SRC_BASE} ${SRC_BUFFER} ${SRC_LOG} ${SRC_MEMORY_POOL} ${SRC_NET} ${SRC_TCP_CONNECTION} ${SRC_TIMER})
target_link_libraries(echo_server  pthread)

add_executable(echo_server_async ${SRC_echo_server_async} ${SRC_BASE} ${SRC_BUFFER} ${SRC_LOG} ${SRC_MEMORY_POOL} ${SRC_NET} ${SRC_TCP_CONNECTION} ${SRC_TIMER})
target_link_libraries(echo_server_async pthread)


###########################
#  unit test
# enable CTest testing 

# add a testing executable