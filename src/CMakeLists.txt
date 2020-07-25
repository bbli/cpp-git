cmake_minimum_required(VERSION 3.10)
# so library will be built first
add_subdirectory(lib)

set(cpg ${CMAKE_PROJECT_NAME})
add_executable(cpg main.cpp)
# target_compile_features(cpg PRIVATE cxx_std_17)

target_link_libraries(cpg PRIVATE command_library)
if (CLANG_TIDY)
    set_target_properties(cpg PROPERTIES CXX_CLANG_TIDY "${CLANG_TIDY_OPTIONS}")
    #set_target_properties(cpg PROPERTIES CXX_CLANG_TIDY "clang-tidy;--checks=*,readability=*")
endif()

install(TARGETS cpg DESTINATION ${INSTALL_BIN_DIR})
