# so library will be built first
add_subdirectory(lib)

set(EXEC ${CMAKE_PROJECT_NAME})
add_executable(EXEC main.cpp)
target_include_directories(EXEC PUBLIC lib)
target_compile_features(EXEC PUBLIC cxx_std_17)

# hopefully cmake will be able to find
target_link_libraries(EXEC PUBLIC helper_library)

install(TARGETS EXEC DESTINATION ${INSTALL_BIN_DIR})
