add_executable(async_logging_test async_logging_test.cc)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/src/logger/test)

target_link_libraries(async_logging_test)

