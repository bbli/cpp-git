#include "git_objects.hpp"
#include "helper.hpp"
#include "commands.hpp"

#include <filesystem>
#include <iostream>
#include <vector>


int test_function(void) {
    std::vector<int> test;
    std::cout << "Hello World" << std::endl;
    return 0;
}

void git_init(fs::path project_base_path) {
    // check that .git doesn't exist or is empty directory
    fs::path git_path = project_base_path / ".cpp-git";
    if (fs::exists(git_path)) {
        throw "init error: Not an empty path";
    }
    // create object dir
    fs::create_directories(git_path / "objects");
    // create HEAD file with "ref: refs/heads/master"
    write_file(git_path / "HEAD", "ref: refs/heads/master");
    // create branches dir
    fs::create_directories(git_path / "branches");
    // create refs dir with tags+heads subdirectory
    fs::create_directories(git_path / "refs" / "tags");
    fs::create_directories(git_path / "refs" / "heads");
}

std::string git_add_file(const fs::path& file_path) {
    fs::path project_base_path = repo_find(file_path);
    fs::path git_path = project_base_path / ".cpp-git";
    GitTree tree_obj(git_path);

    // Run file_it
    auto base_it = project_base_path.begin();
    auto file_it = file_path.begin();
    while (base_it != project_base_path.end()) {
        base_it++;
        file_it++;
    }
    // *file_it=folder in /project_base_path/folder/etc...
    // Paths are unique, but individual names arn't
    // EC: if file is one_level
    // EC: if file is new file
    std::string root_tree_hash = getTreeHashOfIndex(git_path);
    std::cout << "HashOfHead:" << root_tree_hash << std::endl;
    std::string new_tree_hash =
        getSubTreeHashForNewFile(root_tree_hash, file_it, file_path.end(), git_path, file_path);

    std::cout << "Should write to index now" << std::endl;
    write_file(git_path / "index", new_tree_hash);
    return new_tree_hash;
}

std::string git_add_folder(const fs::path folder_path) {
    fs::path project_base_path = repo_find(folder_path);
    fs::path git_path = project_base_path / ".cpp-git";
    GitTree tree_obj(git_path);

    // Run file_it
    auto base_it = project_base_path.begin();
    auto file_it = folder_path.begin();
    while (base_it != project_base_path.end()) {
        base_it++;
        file_it++;
    }

    std::string old_tree_hash = getTreeHashOfIndex(git_path);
    std::cout << "HashOfHead:" << old_tree_hash << std::endl;
    std::string new_tree_hash =
        getSubTreeHashForNewFolder(old_tree_hash, file_it, folder_path.end(), git_path, folder_path);

    std::cout << "Should write to index now" << std::endl;
    write_file(git_path / "index", new_tree_hash);
    return new_tree_hash;
}

