if(Boost_FOUND)
add_executable(idleconnection_echo echo.cc main.cc)
target_link_libraries(idleconnection_echo ${Boost_LIBRARIES} ryan_network)
endif()

add_executable(idleconnection_echo2 sortedlist.cc)
target_link_libraries(idleconnection_echo2 ryan_network)
