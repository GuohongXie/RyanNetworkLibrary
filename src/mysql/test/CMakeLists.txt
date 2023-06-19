# 设置项目名称
project(mysql_conn_pool_unittest C CXX)

# 指定使用 CMake 的最低版本号
cmake_minimum_required(VERSION 3.15)


# 添加搜索路径
include_directories(
    ${PROJECT_SOURCE_DIR}/../../mysql
    )

# 添加线程库
find_package(Threads REQUIRED)

# check if boost was found
#if(Boost_FOUND)
#    message ("boost found")
#else()
#    message (FATAL_ERROR "Cannot find Boost")
#endif()

# 包含这些目录下的.cc文件
# 下面这两个会把目录及其子目录的所有.cc文件都包括
#aux_source_directory(${PROJECT_SOURCE_DIR}/../../mysql SRC_LOG)
#aux_source_directory(${PROJECT_SOURCE_DIR}/../../mysql SRC_LOG)
#如果想要不包含子目录下的文件，则使用file
file(GLOB SRC_mysql ${PROJECT_SOURCE_DIR}/../../mysql/*.cc)

set (SRC_MAIN  ${PROJECT_SOURCE_DIR}/mysql_conn_pool_unittest.cc)

# 设置编译选项
set(CXX_FLAGS
    -g
    -Wall
    -std=c++11
    )


#add an executable with the above sources
add_executable(mysql_conn_pool_unittest ${SRC_MAIN} ${SRC_mysql})
#target_link_libraries(mysql_conn_pool_unittest ${Boost_LIBRARIES} pthread)
target_link_libraries(mysql_conn_pool_unittest PRIVATE Threads::Threads)
# 链接MySQL库
target_include_directories(mysql_conn_pool_unittest PRIVATE /usr/include/mysql)
target_link_libraries(mysql_conn_pool_unittest PRIVATE mysqlclient)




###########################
#  unit test
# enable CTest testing 

# add a testing executable
