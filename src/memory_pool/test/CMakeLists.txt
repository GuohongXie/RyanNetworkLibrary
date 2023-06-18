# 设置项目名称
project(memory_pool_unittest C CXX)

# 指定使用 CMake 的最低版本号
cmake_minimum_required(VERSION 3.15)


# 添加搜索路径
include_directories(
    ${PROJECT_SOURCE_DIR}/../../memory_pool
    )

# 包含这些目录下的.cc文件
# 下面这两个会把目录及其子目录的所有.cc文件都包括
#aux_source_directory(${PROJECT_SOURCE_DIR}/../../logger SRC_LOG)
#aux_source_directory(${PROJECT_SOURCE_DIR}/../../logger SRC_LOG)
#如果想要不包含子目录下的文件，则使用file
file(GLOB SRC_memory_pool ${PROJECT_SOURCE_DIR}/../../memory_pool/*.cc)

set (SRC_MAIN  ${PROJECT_SOURCE_DIR}/memory_pool_unittest.cc)

# 设置编译选项
set(CXX_FLAGS
    -g
    -Wall
    -std=c++11
    )


#add an executable with the above sources
add_executable(memory_pool_unittest ${SRC_MAIN} ${SRC_memory_pool})



###########################
#  unit test
# enable CTest testing 

# add a testing executable
