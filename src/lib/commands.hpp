#ifndef COMMANDS_HPP
#define COMMANDS_HPP

#include <filesystem>
// To get the data structures
#include <helper.hpp>

namespace fs = std::filesystem;


/* ********* Functions	********* */
int test_function(void);
Repo git_init(fs::path git_path);
#endif
