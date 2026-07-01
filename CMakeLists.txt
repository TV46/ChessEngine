cmake_minimum_required(VERSION 4.2)
project(ChessEngine)

set(CMAKE_CXX_STANDARD 23)

add_executable(ChessEngine main.cpp
        frontend.cpp
        frontend.h
        evaluations.cpp
        evaluations.h
        boardSetup.h)
