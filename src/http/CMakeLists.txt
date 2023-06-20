# 加载子目录
set(HTTP_SRCS
  http_server.cc
  http_response.cc
  http_context.cc
  main.cc
)

add_executable(http_server ${HTTP_SRCS})

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/src/http)

target_link_libraries(http_server ryan_network)

