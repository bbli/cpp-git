cmake_minimum_required(VERSION 3.10)
# so library will be built first
add_subdirectory(lib)

set(cpg ${CMAKE_PROJECT_NAME})
add_executable(cpg main.cpp)
# target_include_directories(cpg PUBLIC lib)
# target_compile_features(cpg PUBLIC cxx_std_17)

# hopefully cmake will be able to find
target_link_libraries(cpg PUBLIC command_library)

install(TARGETS cpg DESTINATION ${INSTALL_BIN_DIR})
