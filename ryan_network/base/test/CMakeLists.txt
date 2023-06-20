add_executable(thread_test thread_test.cc)
target_link_libraries(thread_test ryan_network)

#add_executable(timestamp_unittest timestamp_unittest.cc)
#target_link_libraries(timestamp_unittest ryan_network)

add_executable(thread_pool_test thread_pool_test.cc)
target_link_libraries(thread_pool_test ryan_network)



