add_executable(sudoku_solver_basic server_basic.cc sudoku.cc)
target_link_libraries(sudoku_solver_basic ryan_network)

add_executable(sudoku_solver_multiloop server_multiloop.cc sudoku.cc)
target_link_libraries(sudoku_solver_multiloop ryan_network)

add_executable(sudoku_solver_threadpool server_threadpool.cc sudoku.cc)
target_link_libraries(sudoku_solver_threadpool ryan_network)

if(Boost_FOUND)
add_executable(sudoku_solver_hybrid server_hybrid.cc sudoku.cc)
target_link_libraries(sudoku_solver_hybrid ${Boost_LIBRARIES} ryan_network)

add_executable(sudoku_solver_prod server_prod.cc sudoku.cc)
target_link_libraries(sudoku_solver_prod ${Boost_LIBRARIES} ryan_network)
endif()


#add_executable(sudoku_client_batch batch.cc sudoku.cc)
#target_link_libraries(sudoku_client_batch ryan_network)

#add_executable(sudoku_client_pipeline pipeline.cc sudoku.cc)
#target_link_libraries(sudoku_client_pipeline ryan_network)

#add_executable(sudoku_loadtest loadtest.cc sudoku.cc)
#target_link_libraries(sudoku_loadtest ryan_network)


#if(BOOSTTEST_LIBRARY)
#add_executable(sudoku_stat_unittest stat_unittest.cc)
#target_link_libraries(sudoku_stat_unittest muduo_base boost_unit_test_framework)
#endif()

