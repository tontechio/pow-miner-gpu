PROJECT(win_gettimeofday)
cmake_minimum_required(VERSION 3.0.2 FATAL_ERROR)

set(WIN_USLEEP_SOURCE win_gettimeofday.hpp win_gettimeofday.cpp)
add_library(win_gettimeofday ${WIN_USLEEP_SOURCE})
target_include_directories(win_gettimeofday PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)
