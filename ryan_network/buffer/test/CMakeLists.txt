if(BOOST_TEST_LIBRARY)
add_executable(buffer_unittest buffer_unittest.cc)
target_link_libraries(buffer_unittest pthread boost_unit_test_framework)
add_test(NAME buffer_unittest COMMAND buffer_unittest)
endif()

