#ifndef COMMANDS_HPP
#define COMMANDS_HPP

#include <filesystem>
// To get the data structures
#include <git_objects.hpp>
/* #include <helper.hpp> */

namespace fs = std::filesystem;

/* ********* Functions	********* */
int test_function(void);
void git_init(fs::path project_base_path);

std::string git_add_file(const fs::path& file_path);
std::string git_add_folder(const fs::path folder_path);
#endif
