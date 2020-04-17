#include <vector>
#include <iostream>
#include <filesystem>

#include "helper.hpp"
#include "commands.hpp"

int test_function(void){
    std::vector<int> test;
    std::cout << "Hello World" << std::endl;
    return 0;
}

void git_init(void){
    fs::path git_path = fs::current_path();
    // check that .git doesn't exist or is empty directory
    git_path /= ".cpp-git";
    if (fs::exists(git_path)){
        throw "init error: Not an empty path";
    }
    // initialize a repo object
    /* Repo repo(source_path,true); */

    //create object dir
    fs::create_directories(git_path / "objects");
    //create HEAD file with "ref: refs/heads/master"
    write_file(git_path / "heads" / "master","ref: refs/heads/master");
    //create branches dir
    fs::create_directories(git_path / "branches");
    //create refs dir with tags+heads subdirectory
    fs::create_directories(git_path / "refs" / "tags");
    fs::create_directories(git_path / "refs" / "heads");

}

// BC: For now, can only add from project root
// since initial git_add will start on project's root
// and add all blobs relative to it
std::string git_add(const fs::path& adding_directory, bool root){
    fs::path git_path = repo_find(adding_directory) / ".cpp-git";

    GitTree tree_obj;
    for (auto entry: fs::directory_iterator(adding_directory)){
        fs::path path = entry.path();
        std::cout << "Entry path: " << path << std::endl;
        if (fs::is_regular_file(entry)){
            // read in and make GitBlob
            std::string content = read_file(path);
            GitBlob blob_obj = GitBlob(git_path,content);
            // write to object folder
            std::string blob_hash = writeObject(&blob_obj);
            // append to tree object
            tree_obj.add_entry("blob",path.filename(),blob_hash);
        }
        else if (fs::is_directory(path)){
            std::string tree_hash = git_add(path);
            tree_obj.add_entry("tree",path.filename(),tree_hash);
        }
        else {
            throw "cpp-git cannot handle this file";
        }
    }
    std::string output =  writeObject(&tree_obj);
    if (root){
        std::cout << "Should write to index now" << std::endl;
        write_file(git_path / "index",output);
    }
    return output;
}
