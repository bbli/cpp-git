cmake_minimum_required(VERSION 3.1)
file(GLOB helpers "*.cpp" )
add_library(helper_library ${helpers})
install(TARGETS helper_library DESTINATION ${PROJECT_SOURCE_DIR}/lib/${CMAKE_PROJECT_NAME})
