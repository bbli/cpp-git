cmake_minimum_required(VERSION 3.10)
# so library will be built first
add_subdirectory(lib)

set(E ${CMAKE_PROJECT_NAME})
add_executable(E main.cpp)
# target_include_directories(E PUBLIC lib)
# target_compile_features(E PUBLIC cxx_std_17)

# hopefully cmake will be able to find
target_link_libraries(E PUBLIC command_library)

install(TARGETS E DESTINATION ${INSTALL_BIN_DIR})
