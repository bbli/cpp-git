#include "git_objects.hpp"
#include "helper.hpp"
#include "commands.hpp"

#include <string>
#include <filesystem>
#include <iostream>
#include <vector>
#include <unordered_map>
#include <set>

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
    write_file(git_path / "index", "");
    // create branches dir
    fs::create_directories(git_path / "branches");
    // create refs dir with tags+heads subdirectory
    fs::create_directories(git_path / "refs" / "tags");
    fs::create_directories(git_path / "refs" / "heads");
}

std::string git_add_file(const fs::path& file_path) {
    fs::path project_base_path = repo_find(file_path);
    fs::path git_path = project_base_path / ".cpp-git";
    std::string new_tree_hash;

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
    GitTree* index_tree = getIndexTree(git_path);
    if (!index_tree){
        // then base off head tree instead
        // if even head is empty, just add from project folder instead of traversing git trees
        /* std::cout << "got past index" << std::endl; */
        GitTree* head_tree = get_head_tree(git_path);
        /* std::cout << "got past head" << std::endl; */
        if (!head_tree){
            /* std::cout << "got here" << std::endl; */
            // TODO: check that folder path and project base path are the same
            std::string blob_hash = readProjectFileAndWriteObject(git_path,file_path);
            GitTree tree_obj(git_path);
            tree_obj.add_entry("blob",file_path.filename(),blob_hash);
            new_tree_hash = writeObject(&tree_obj);
            write_file(git_path / "index",new_tree_hash);
        }
        else{
            new_tree_hash = getSubTreeHashForNewFile(head_tree,file_it,file_path.end(),git_path,file_path);
        }
    }
    else{
        new_tree_hash =
            getSubTreeHashForNewFile(index_tree, file_it, file_path.end(), git_path, file_path);
    }

    std::cout << "Should write to index now" << std::endl;
    write_file(git_path / "index", new_tree_hash);
    return new_tree_hash;
}

std::string git_add_folder(const fs::path folder_path) {
    fs::path project_base_path = repo_find(folder_path);
    fs::path git_path = project_base_path / ".cpp-git";
    std::string new_tree_hash;

    // Run file_it
    auto base_it = project_base_path.begin();
    auto file_it = folder_path.begin();
    while (base_it != project_base_path.end()) {
        base_it++;
        file_it++;
    }

    GitTree* index_tree = getIndexTree(git_path);
    if (!index_tree){
        // TODO: check that folder path and project base path are the same
        new_tree_hash = readProjectFolderAndWriteTree(project_base_path);
    }
    else{
        new_tree_hash =
            getSubTreeHashForNewFolder(index_tree, file_it, folder_path.end(), git_path, folder_path);
    }

    std::cout << "Should write to index now" << std::endl;
    write_file(git_path / "index", new_tree_hash);
    return new_tree_hash;
}

std::vector<std::string> tree_hash_to_listing_hash(std::string tree_hash,fs::path git_path){
    std::vector<GitTreeNode> listing = dynamic_cast<GitTree*>(readObject(git_path,tree_hash))->directory;
    std::vector<std::string> result;
    std::transform(listing.begin(),listing.end(),std::back_inserter(result),[](auto x){return x.hash;});
    return result;
}

std::set<std::string> set_difference_of_trees(std::string index_tree_hash, std::string head_tree_hash,fs::path git_path){
    std::vector<std::string> index_first_level_hashes = tree_hash_to_listing_hash(index_tree_hash,git_path);
    std::vector<std::string> head_first_level_hashes = tree_hash_to_listing_hash(head_tree_hash,git_path);

    std::sort(index_first_level_hashes.begin(),index_first_level_hashes.end());
    std::sort(head_first_level_hashes.begin(),head_first_level_hashes.end());

    std::set<std::string> new_hashes;
    std::set_difference(index_first_level_hashes.begin(),index_first_level_hashes.end(),head_first_level_hashes.begin(),head_first_level_hashes.end(),std::back_inserter(new_hashes));
    return new_hashes;
}

void git_status(const fs::path git_path){
    GitTree* index_tree = getIndexTree(git_path);
    GitTree* head_tree = get_head_tree(git_path);

    auto new_index_hashes = set_difference_of_trees(index_tree_hash,head_tree_hash,git_path);

    // Now walk through index tree listing and print out the new_hashes
}
