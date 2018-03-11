cmake_minimum_required(VERSION 3.9)
project(Projekt1)

set(CMAKE_CXX_STANDARD 14)
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -lws2_32" )

add_executable(Projekt1 ipk-client.cpp ipk-server.cpp ipk-shared.cpp)
