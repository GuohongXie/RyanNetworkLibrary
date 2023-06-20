# 指定使用 CMake 的最低版本号
cmake_minimum_required(VERSION 3.0)

# 设置项目名称
project(ryan_network C CXX)

# 添加搜索路径
include_directories(
    ${PROJECT_SOURCE_DIR}/ryan_network/base
    ${PROJECT_SOURCE_DIR}/ryan_network/net
    ${PROJECT_SOURCE_DIR}/ryan_network/tcp_connection
    ${PROJECT_SOURCE_DIR}/ryan_network/http
    ${PROJECT_SOURCE_DIR}/ryan_network/timer
    ${PROJECT_SOURCE_DIR}/ryan_network/logger
    ${PROJECT_SOURCE_DIR}/ryan_network/memory_pool
    ${PROJECT_SOURCE_DIR}/ryan_network/mysql
    )


# 包含这些目录下的.cc文件
# 下面这两个会把目录及其子目录的所有.cc文件都包括
#aux_source_directory(${PROJECT_SOURCE_DIR}/../../logger SRC_LOGGER)
#aux_source_directory(${PROJECT_SOURCE_DIR}/../../logger SRC_LOGGER)
#如果想要不包含子目录下的文件，则使用file
file(GLOB SRC_BASE ${PROJECT_SOURCE_DIR}/../../base/*.cc)
file(GLOB SRC_BUFFER ${PROJECT_SOURCE_DIR}/../../buffer/*.cc)
file(GLOB SRC_HTTP ${PROJECT_SOURCE_DIR}/../../http/*.cc)
file(GLOB SRC_LOGGER ${PROJECT_SOURCE_DIR}/../../logger/*.cc)
file(GLOB SRC_MEMORY_POOL ${PROJECT_SOURCE_DIR}/../../memory_pool/*.cc)
file(GLOB SRC_MYSQL ${PROJECT_SOURCE_DIR}/../../mysql/*.cc)
file(GLOB SRC_NET ${PROJECT_SOURCE_DIR}/../../net/*.cc)
file(GLOB SRC_TCP_CONNECTION ${PROJECT_SOURCE_DIR}/../../tcp_connection/*.cc)
file(GLOB SRC_TIMER ${PROJECT_SOURCE_DIR}/../../timer/*.cc)

# 设置编译选项
set(CXX_FLAGS
    -g
    -Wall
    -std=c++11
    )

set(CMAKE_CXX_FLAGS_DEBUG "-O0")
set(CMAKE_CXX_FLAGS_RELEASE "-O2 -DNDEBUG")
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

find_package(Boost REQUIRED)
find_library(BOOST_TEST_LIBRARY NAMES boost_unit_test_framework)
include_directories(${Boost_INCLUDE_DIRS})

# 生成动态库 ryan_network
add_library(ryan_network SHARED 
            ${SRC_BASE}
            ${SRC_BUFFER}
            ${SRC_HTTP}
            ${SRC_LOGGER}
            ${SRC_MEMORY_POOL}
            ${SRC_MYSQL}
            ${SRC_NET}
            ${SRC_TCP_CONNECTION}
            ${SRC_TIMER}
            )

# 目标动态库所需连接的库（这里需要连接libpthread.so）
target_link_libraries(ryan_network pthread mysqlclient)

# 设置生成动态库的路径，放在根目录的lib文件夹下面
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

# 设置安装的默认路径

# 将目录安装到 include/ryan_network 下（只安装头文件）
#install(DIRECTORY ${PROJECT_SOURCE_DIR}/ryan_network/ DESTINATION build/include/ryan_network FILES_MATCHING PATTERN "*.h")

# 加载example
add_subdirectory(examples/echo_server)

#加载test
add_subdirectory(ryan_network/base/test)
add_subdirectory(ryan_network/bufer/test)
add_subdirectory(ryan_network/logger/test)
add_subdirectory(ryan_network/memory_pool/test)
add_subdirectory(ryan_network/net/test)
add_subdirectory(ryan_network/timer/test)