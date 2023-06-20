if(BOOST_TEST_LIBRARY)
add_executable(inet_address_unittest inet_address_unittest.cc)
target_link_libraries(inet_address_unittest ryan_network boost_unit_test_framework)
add_test(NAME inet_address_unittest COMMAND inet_address_uittest)
endif()

add_executable(event_loop_unittest event_loop_unittest.cc)
target_link_libraries(event_loop_unittest ryan_network)

#add_executable(event_loop_thread_unittest event_loop_thread_unittest.cc)
#target_link_libraries(event_loop_thread_unittest ryan_network)

add_executable(event_loop_thread_pool_unittest event_loop_thread_pool_unittest.cc)
target_link_libraries(event_loop_thread_pool_unittest ryan_network)


###########################
#  unit test
# enable CTest testing 

# add a testing executable
