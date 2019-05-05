cmake_minimum_required(VERSION 3.14)
project(SIK_ZAD2)

set(CMAKE_CXX_STANDARD 17)

add_executable(SIK_ZAD2 src/client.cpp src/server.cpp src/common/const.h src/server/server.hpp src/common/helper.hpp src/client/client.hpp src/common/message.hpp src/server/folder_handler.hpp src/server/server_socket.hpp src/common/type.hpp)