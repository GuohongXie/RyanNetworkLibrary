add_executable(async_logging_test_1 async_logging_test_1.cc)
target_link_libraries(async_logging_test_1 ryan_network)

add_executable(async_logging_test_2 async_logging_test_2.cc)
target_link_libraries(async_logging_test_2 ryan_network)

add_executable(log_file_test log_file_test.cc)
target_link_libraries(log_file_test ryan_network)

add_executable(log_stream_bench log_stream_bench.cc)
target_link_libraries(log_stream_bench ryan_network)


add_executable(logging_test logging_test.cc)
target_link_libraries(logging_test ryan_network)