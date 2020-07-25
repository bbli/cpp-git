cmake_minimum_required(VERSION 3.10)
file(GLOB helpers CONFIGURE_DEPENDS "*.cpp")
add_library(command_library ${helpers})
# add_library(command_library SHARED ${helpers})
# target_compile_options(command_library PUBLIC -fPIC)
include(CMakePrintHelpers)
if (CLANG_TIDY)
    set_target_properties(command_library PROPERTIES CXX_CLANG_TIDY "${CLANG_TIDY_OPTIONS}")
    #set_target_properties(command_library PROPERTIES CXX_CLANG_TIDY "clang-tidy;--checks=*,readability=*")
    #cmake_print_variables(CLANG_TIDY_OPTIONS)
endif()
target_include_directories(command_library PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
# Private since sha1 only used internally
target_link_libraries(command_library PRIVATE sha1_library)
# target_link_libraries(command_library PUBLIC range_v3)
install(TARGETS command_library DESTINATION ${PROJECT_SOURCE_DIR}/lib/${CMAKE_PROJECT_NAME})

#cmake_print_variables(CMAKE_INSTALL_PREFIX)
#cmake_print_properties(
    #TARGETS command_library
    #PROPERTIES POSITION_INDEPENDENT_CODE
#)
