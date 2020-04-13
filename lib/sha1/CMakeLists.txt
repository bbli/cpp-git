cmake_minimum_required(VERSION 3.10)
add_library(sha1_library sha1.cpp)
target_include_directories(sha1_library PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
