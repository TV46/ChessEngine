cmake_minimum_required(VERSION 4.2)
project(ChessEngine)

find_package(SFML 3 COMPONENTS Graphics Window System REQUIRED)



set(CMAKE_CXX_STANDARD 23)

add_executable(ChessEngine main.cpp
)
