# 指定使用 CMake 的最低版本号
cmake_minimum_required(VERSION 3.15)

# 设置项目名称
project(logger_test C CXX)

# 添加搜索路径
include_directories(
    ${PROJECT_SOURCE_DIR}/../../base
    ${PROJECT_SOURCE_DIR}/../../logger
    )

#
set (SRC_THREAD
     ../../base/thread.cc
     ../../base/current_thread.cc
)
set (SRC_TIMESTAMP ../../base/timestamp.cc)
set (SRC_MAIN async_logging_test.cc)

# 包含这些目录下的.cc文件
aux_source_directory(${PROJECT_SOURCE_DIR}/../../logger SRC_LOG)


# 设置编译选项
set(CXX_FLAGS
    -g
    -Wall
    -std=c++17
    )


#add an executable with the above sources
add_executable(logger_test ${SRC_MAIN} ${SRC_TIMESTAMP} ${SRC_LOG} ${SRC_THREAD})


target_link_libraries(logger_test pthread)