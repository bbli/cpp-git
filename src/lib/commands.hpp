#ifndef COMMANDS_HPP
#define COMMANDS_HPP

#include <filesystem>
#include <set>
#include <unordered_map>
// To get the data structures
#include "git_objects.hpp"

/* ********* Global Variables ********** */
const std::set<std::string> CAT_FILE_SUBCMDS = {"blob", "commit", "tag", "tree"};
#define CAT_FILE_USAGE \
    "usage: git cat-file <type> <object>\n<type> can be one of: blob, tree, commit, tag"
#define HASH_OBJECT_USAGE \
    "usage: git hash-object <type> <path>\n<type> can be one of: blob, tree, commit, tag"
#define GIT_INIT_USAGE \
    "usage: git init <dir>\nCreate an empty Git repository as <dir>. <dir> defaults to be '.'"
#define CHECKOUT_USAGE "usage: git checkout [<branch>/<commit>]"
#define LOG_USAGE "usage: git log [-n <number>]"

namespace fs = std::filesystem;

/* ********* Functions	********* */
// Functions dealing with commands: validate, convert file path to absolute etc.
void cmd_init(const std::vector<std::string>& args);
void cmd_add(const std::vector<std::string>& args);
void cmd_cat_file(const std::vector<std::string>& args);
void cmd_checkout(const std::vector<std::string>& args);
void cmd_commit(const std::vector<std::string>& args);
void cmd_branch(const std::vector<std::string>& args);
void cmd_show_ref(const std::vector<std::string>& args);
void cmd_hash_object(const std::vector<std::string>& args);
void cmd_reset(const std::vector<std::string>& args);
void cmd_tag(const std::vector<std::string>& args);
void cmd_log(const std::vector<std::string>& args);
void cmd_status(const std::vector<std::string>& args);
void cmd_clean(const std::vector<std::string>& args);
// Actual functions executing commands
void git_cat_file(fs::path obj, const std::string& fmt);
void git_init(fs::path project_base_path);
void git_commit(std::string commit_message);
void git_show_ref(const std::string& prefix = "");
void git_hash_object(fs::path path, const std::string& fmt);

void git_reset_project(bool hard);
std::string git_reset_file(fs::path file_path, bool hard);
void git_create_tag(std::string name, std::string object, bool if_create_object,
                    std::string tag_message = "");
void git_log(int num);
void git_checkout_file(fs::path file_path);
void git_checkout_branch(std::string branch_name);
void git_branch_new(std::string branch_name);
void git_branch_delete(std::string branch_name);
void git_branch_list(void);
std::string git_add_file(const fs::path& file_path);
std::string git_add_folder(const fs::path folder_path);
void git_status_commit_index(void);
void git_status_index_vs_project(void);

int test_function(void);

#endif
