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




void get_project_file_hashes(fs::path directory,std::vector<std::string>& project_leaf_hashes,std::unordered_map<std::string,std::string>& path_to_hash_dict, const fs::path git_path){
    for (auto entry: fs::directory_iterator(directory)){
        fs::path entry_path = entry.path();
        if (fs::is_regular_file(entry_path)){
            std::string file_hash = readProjectFileAndWriteObject(git_path,entry_path);
            project_leaf_hashes.push_back(file_hash);
            path_to_hash_dict.insert({entry_path.string(),file_hash});
        }
        else if (fs::is_directory(entry_path)){
            get_project_file_hashes(entry_path,project_leaf_hashes,path_to_hash_dict,git_path);
        }
        else{
            throw "cpp-git cannot handle this file";
        }
    }
}



/* void print_new_hashes(fs::path directory, const std::set<std::string>& diff_hashes,const std::unordered_map<std::string,std::string>& path_to_hash_dict,const fs::path project_base_path){ */
/*     std::cout << "---------------Unstaged changes------------" << std::endl; */
/*     for (auto entry: fs::directory_iterator(directory)){ */
/*         fs::path entry_path = entry.path(); */
/*         auto cached_hash = path_to_hash_dict.at(entry_path.string()); */
/*         if (fs::is_regular_file(entry_path)){ */
/*             bool found = isInSet(diff_hashes,cached_hash); */
/*             if (found){ */
/*                 std::cout << "new/modified file: " << path_relative_to_project(project_base_path,entry_path) << std::endl; */
/*             } */
/*         } */
/*         else if (fs::is_directory(entry_path)){ */
/*             print_new_hashes(entry_path,diff_hashes,path_to_hash_dict,project_base_path); */
/*         } */
/*         else{ */
/*             throw "cpp-git cannot handle this file"; */
/*         } */
/*     } */
/* } */

std::set<std::string> set_difference_of_leaf_hashes(std::vector<std::string>& leaf_hashes_1, std::vector<std::string>& leaf_hashes_2){
    std::sort(leaf_hashes_1.begin(),leaf_hashes_1.end());
    std::sort(leaf_hashes_2.begin(),leaf_hashes_2.end());

    std::vector<std::string> diff_hashes;
    std::set_difference(leaf_hashes_1.begin(),leaf_hashes_1.end(),leaf_hashes_2.begin(),leaf_hashes_2.end(),std::back_inserter(diff_hashes));
    return std::set<std::string>(diff_hashes.begin(),diff_hashes.end());
}

/* ********* 	********* */
void get_leaf_hashes_of_tree(GitTree* tree_obj,std::set<std::string>& index_leaf_hashes, const fs::path git_path){
    for (auto node: tree_obj->directory){
        if (node.type=="blob"){
            index_leaf_hashes.insert(node.hash);
        }
        else if (node.type == "tree"){
            GitTree* subtree = get_tree(node.hash,git_path);
            get_leaf_hashes_of_tree(subtree,index_leaf_hashes,git_path);
        }
        else{
            throw "cpp-git cannot handle this file";
        }
    }
}

void print_unstaged_project_files(fs::path directory, const std::set<std::string>& index_leaf_hashes, const fs::path git_path, const fs::path project_base_path){
    for(auto project_file: fs::directory_iterator(directory)){
        fs::path project_file_path = project_file.path();
        /* std::cout << "file path: " << project_file_path << std::endl; */
        if(isGitRepo(project_file_path)){
            continue;
        }

        if (fs::is_regular_file(project_file_path)){
            std::string file_hash = readProjectFileAndWriteObject(git_path,project_file_path);
            if (!is_in_set(index_leaf_hashes,file_hash)){
                std::cout << "new/modified: " <<  path_relative_to_project(project_base_path,project_file_path) << std::endl;
            }
        }
        else if (fs::is_directory(project_file_path)){
            print_unstaged_project_files(project_file_path,index_leaf_hashes,git_path,project_base_path);
        }
        else{
            throw "cpp-git cannot handle this file";
        }
    }
}

void print_new_index_nodes_and_calc_delete(GitTree* index_tree,std::set<std::string>& delete_hashes, const std::set<std::string>& head_leaf_hashes,const fs::path git_path){
    /* std::cout << "Listing:" << std::endl; */
    /* printer(index_tree->directory); */
    for (auto index_node: index_tree->directory){
        if (index_node.type == "blob"){
            bool found = is_in_set(head_leaf_hashes,index_node.hash);
            // This is not in commit set, so print
            if (!found){
                std::cout << "new/modified file: " << index_node.name << std::endl;
            }
            // This node is in the commit set, so should not exist in delete_hashes
            else{
                delete_hashes.erase(index_node.hash);
            }
        }
        else if (index_node.type == "tree"){
            GitTree* subtree = get_tree(index_node.hash,git_path);
            print_new_index_nodes_and_calc_delete(subtree,delete_hashes,head_leaf_hashes,git_path);
        }
    }
}

void print_deleted_head_nodes(GitTree* head_tree, const std::set<std::string>& delete_hashes, fs::path rel_path, const fs::path git_path){
    for (auto head_node: head_tree->directory){
        if (head_node.type == "blob"){
            bool deleted = is_in_set(delete_hashes,head_node.hash);
            if (deleted){
                std::cout << "deleted: " << (rel_path / head_node.name) << std::endl;
            }
        }
        else if (head_node.type == "tree"){
            GitTree* subtree = get_tree(head_node.hash, git_path);
            print_deleted_head_nodes(subtree,delete_hashes,rel_path / head_node.name, git_path);
        }
        else{
            throw "cpp-git cannot handle this file";
        }
    }
}

void git_status_index_vs_project(const fs::path git_path){
    fs::path project_base_path = repo_find(git_path);

    GitTree* index_tree = getIndexTree(git_path);
    std::set<std::string> index_leaf_hashes;
    get_leaf_hashes_of_tree(index_tree,index_leaf_hashes,git_path);

    std::cout << "---------------Files not yet staged------------" << std::endl;
    print_unstaged_project_files(project_base_path,index_leaf_hashes,git_path,project_base_path);
}

void git_status_commit_index(const fs::path git_path){
    GitTree* index_tree = getIndexTree(git_path);
    GitTree* head_tree = get_head_tree(git_path);
    if (!index_tree || !head_tree){
        std::cout << "Something went wrong" << std::endl;
    }
    std::set<std::string> head_leaf_hashes;
    get_leaf_hashes_of_tree(head_tree,head_leaf_hashes,git_path);

    std::set<std::string> delete_hashes = head_leaf_hashes;
    std::cout << "---------------Files staged for commit:------------" << std::endl;
    print_new_index_nodes_and_calc_delete(index_tree,delete_hashes,head_leaf_hashes,git_path);
    print_deleted_head_nodes(head_tree,delete_hashes,"",git_path);
}
