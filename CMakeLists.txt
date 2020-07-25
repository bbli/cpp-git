cmake_minimum_required(VERSION 3.10)
project(cpp-git)

set(CMAKE_INSTALL_PREFIX ${PROJECT_SOURCE_DIR})
set(INSTALL_BIN_DIR ${PROJECT_SOURCE_DIR}/bin)
set(CMAKE_CXX_STANDARD 20)
#set(CMAKE_BUILD_TYPE Debug)
#set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")
#set (CMAKE_LINKER_FLAGS_DEBUG "${CMAKE_LINKER_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")
add_compile_options(-W -Wall)
#if(APPLE)
#    include_directories(/Library/Developer/CommandLineTools/SDKs/MacOSX10.15.sdk/usr/include)
#endif()

# Global Variables
option(CLANG_TIDY "runs clang tidy on everything except the tests" ON)
set(CLANG_TIDY_OPTIONS "clang-tidy;--checks=*,readability=*")

option(CLANG_TIDY "runs clang tidy on everything except the tests" ON)
set(CLANG_TIDY_OPTIONS "clang-tidy;--checks=*,readability=*")

set(CMAKE_EXPORT_COMPILE_COMMANDS YES)

add_subdirectory(src)
add_subdirectory(lib/sha1)

option(COMPILE_TESTS "Build the tests" ON)
if(COMPILE_TESTS)
    enable_testing()
    add_subdirectory(lib/gtest)
    add_subdirectory(test)
    add_test(NAME test COMMAND TEST)
endif()


# SANTIZERS
# Build Types
set(CMAKE_BUILD_TYPE ${CMAKE_BUILD_TYPE}
    CACHE STRING "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel tsan asan lsan msan ubsan"
    FORCE)

# ThreadSanitizer
set(CMAKE_CXX_FLAGS_TSAN
    "-fsanitize=thread -g -O1"
    CACHE STRING "Flags used by the C++ compiler during ThreadSanitizer builds."
    FORCE)

# AddressSanitize
set(CMAKE_CXX_FLAGS_ASAN
    "-fsanitize=address -fno-optimize-sibling-calls -fsanitize-address-use-after-scope -fno-omit-frame-pointer -g -O1"
    CACHE STRING "Flags used by the C++ compiler during AddressSanitizer builds."
    FORCE)

# LeakSanitizer
set(CMAKE_CXX_FLAGS_LSAN
    "-fsanitize=leak -fno-omit-frame-pointer -g -O1"
    CACHE STRING "Flags used by the C++ compiler during LeakSanitizer builds."
    FORCE)

# MemorySanitizer
set(CMAKE_CXX_FLAGS_MSAN
    "-fsanitize=memory -fno-optimize-sibling-calls -fsanitize-memory-track-origins=2 -fno-omit-frame-pointer -g -O2"
    CACHE STRING "Flags used by the C++ compiler during MemorySanitizer builds."
    FORCE)

# UndefinedBehaviour
set(CMAKE_CXX_FLAGS_UBSAN
    "-fsanitize=undefined"
    CACHE STRING "Flags used by the C++ compiler during UndefinedBehaviourSanitizer builds."
    FORCE)

