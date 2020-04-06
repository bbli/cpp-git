cmake_minimum_required(VERSION 3.10)
project(cpp-git)

set(CMAKE_INSTALL_PREFIX ${PROJECT_SOURCE_DIR})
set(INSTALL_BIN_DIR ${PROJECT_SOURCE_DIR}/bin)
add_compile_options(-W -Wall -Werror)
set(CMAKE_CXX_STANDARD 17)
if(APPLE)
    include_directories(/Library/Developer/CommandLineTools/SDKs/MacOSX10.15.sdk/usr/include)
endif()

add_subdirectory(src)
add_subdirectory(lib/gtest)
add_subdirectory(test)
