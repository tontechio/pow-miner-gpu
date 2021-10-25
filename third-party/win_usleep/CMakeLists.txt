PROJECT(win_usleep)
cmake_minimum_required(VERSION 3.0.2 FATAL_ERROR)

set(WIN_USLEEP_SOURCE win_usleep.hpp win_usleep.cpp)
add_library(win_usleep ${WIN_USLEEP_SOURCE})
target_include_directories(win_usleep PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)
