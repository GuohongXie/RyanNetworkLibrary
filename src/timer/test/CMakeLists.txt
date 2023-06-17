cmake_minimum_required(3.15)

project(timer_queue_unittest)

# 添加搜索路径
include_directories(
    ${PROJECT_SOURCE_DIR}/../../base
    ${PROJECT_SOURCE_DIR}/../../logger
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
    #aux_source_directory(${PROJECT_SOURCE_DIR}/../../logger SRC_LOG)
    #aux_source_directory(${PROJECT_SOURCE_DIR}/../../logger SRC_LOG)
    #如果想要不包含子目录下的文件，则使用file
    file(GLOB SRC_LOG ${PROJECT_SOURCE_DIR}/../../logger/*.cc)
    file(GLOB SRC_BUFFER ${PROJECT_SOURCE_DIR}/../../buffer/*.cc)
    
    set (SRC_MAIN  ${PROJECT_SOURCE_DIR}/buffer_unittest.cc)
    
    # 设置编译选项
    set(CXX_FLAGS
        -g
        -Wall
        -std=c++11
        )
    
    
    #add an executable with the above sources
    add_executable(buffer_unittest ${SRC_MAIN} ${SRC_LOG} ${SRC_BUFFER})
    
    
    target_link_libraries(buffer_unittest Boost::boost)