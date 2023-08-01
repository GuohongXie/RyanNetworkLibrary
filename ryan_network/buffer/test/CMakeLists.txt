if(BOOST_TEST_LIBRARY)
add_executable(buffer_unittest_boost buffer_unittest_boost.cc)
target_link_libraries(buffer_unittest_boost pthread boost_unit_test_framework)
add_test(NAME buffer_unittest_boost COMMAND buffer_unittest_boost)
endif()

if(GTest_FOUND)
add_executable(buffer_unittest_gtest buffer_unittest_gtest.cc)
target_link_libraries(buffer_unittest_gtest ${GTEST_BOTH_LIBRARIES} pthread)
add_test(NAME buffer_unittest_gtest COMMAND buffer_unittest_gtest)
endif()

