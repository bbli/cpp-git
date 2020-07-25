cmake_minimum_required(VERSION 3.10)
# where to find the google test library?
# add_library(gtest lib/gtest)
#file(GLOB sources "*.cpp" )
# add_executable(TEST ${sources})

add_executable(TEST test.cpp)
#set_target_properties(TEST PROPERTIES CXX_CLANG_TIDY "clang-tidy;--checks=*,-readability=*")
# so test can find helper.hpp
# target_include_directories(TEST PUBLIC ${PROJECT_SOURCE_DIR}/src/lib)
# target_include_directories(TEST PUBLIC ${PROJECT_SOURCE_DIR}/lib/gtest/googletest/include)
# target_compile_features(TEST PUBLIC cxx_std_17)

target_link_libraries(TEST PRIVATE command_library gtest_main sha1_library)
# install(TARGETS TEST DESTINATION ${INSTALL_BIN_DIR})
