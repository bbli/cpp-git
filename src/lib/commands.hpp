#ifndef COMMANDS_HPP
#define COMMANDS_HPP

#include <filesystem>
// To get the data structures
#include <helper.hpp>

namespace fs = std::filesystem;


/* ********* Functions	********* */
int test_function(void);
void git_init(void);
std::string git_add(const fs::path& adding_directory, bool root=false);
#endif
