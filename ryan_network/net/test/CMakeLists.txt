if(BOOST_TEST_LIBRARY)
add_executable(inet_address_unittest_boost inet_address_unittest_boost.cc)
target_link_libraries(inet_address_unittest_boost ryan_network boost_unit_test_framework)
add_test(NAME inet_address_unittest_boost COMMAND inet_address_unittest_boost)
endif()

if(GTest_FOUND)
add_executable(inet_address_unittest_gtest inet_address_unittest_gtest.cc)
target_link_libraries(inet_address_unittest_gtest ${GTEST_BOTH_LIBRARIES} ryan_network pthread)
add_test(NAME inet_address_unittest_gtest COMMAND inet_address_unittest_gtest)
endif()

add_executable(event_loop_test event_loop_test.cc)
target_link_libraries(event_loop_test ryan_network)

add_executable(event_loop_thread_test event_loop_thread_test.cc)
target_link_libraries(event_loop_thread_test ryan_network)

add_executable(event_loop_thread_pool_test event_loop_thread_pool_test.cc)
target_link_libraries(event_loop_thread_pool_test ryan_network)

add_executable(channel_test channel_test.cc)
target_link_libraries(channel_test ryan_network)

###########################
#  unit test
# enable CTest testing 

# add a testing executable
