cmake_minimum_required(VERSION 3.1)
# where to find the google test library?
# add_library(gtest lib/gtest)
file(GLOB sources "*.cpp" )
add_executable(TEST ${sources})
# so test can find helper.hpp
target_include_directories(TEST PUBLIC ${PROJECT_SOURCE_DIR}/src/lib)
target_include_directories(TEST PUBLIC ${PROJECT_SOURCE_DIR}/lib/gtest/googletest/include)
target_compile_features(TEST PUBLIC cxx_std_17)

target_link_libraries(TEST PUBLIC helper_library gtest)
install(TARGETS TEST DESTINATION ${INSTALL_BIN_DIR})
