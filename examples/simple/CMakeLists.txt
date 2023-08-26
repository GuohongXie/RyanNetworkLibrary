add_executable(simple_chargen chargen/chargen.cc chargen/main.cc)
target_link_libraries(simple_chargen ryan_network)

add_executable(simple_daytime daytime/daytime.cc daytime/main.cc)
target_link_libraries(simple_daytime ryan_network)

add_executable(simple_discard discard/discard.cc discard/main.cc)
target_link_libraries(simple_discard ryan_network)

add_executable(simple_echo echo/echo.cc echo/main.cc)
target_link_libraries(simple_echo ryan_network)

add_executable(simple_time time/time.cc time/main.cc)
target_link_libraries(simple_time ryan_network)

add_executable(simple_allinone
  allinone/allinone.cc
  chargen/chargen.cc
  daytime/daytime.cc
  discard/discard.cc
  echo/echo.cc
  time/time.cc
  )
target_link_libraries(simple_allinone ryan_network)


