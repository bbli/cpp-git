cmake_minimum_required(VERSION 3.1)
project(cpp-git)

set(CMAKE_INSTALL_PREFIX ${PROJECT_SOURCE_DIR})
set(INSTALL_BIN_DIR ${PROJECT_SOURCE_DIR}/bin)
add_compile_options(-W -Wall -Werror)

add_subdirectory(src)
add_subdirectory(lib/gtest)
add_subdirectory(test)
