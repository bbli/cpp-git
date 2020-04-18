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
    write_file(git_path / "HEAD","ref: refs/heads/master");
    //create branches dir
    fs::create_directories(git_path / "branches");
    //create refs dir with tags+heads subdirectory
    fs::create_directories(git_path / "refs" / "tags");
    fs::create_directories(git_path / "refs" / "heads");

}

std::string readFileAndWriteObject(const fs::path& file_path){
    fs::path git_path = repo_find(fs::current_path()) / ".cpp-git";
    // read in and make GitBlob
    std::string content = read_file(file_path);
    GitBlob blob_obj = GitBlob(git_path,content);
    // write to object folder
    return writeObject(&blob_obj);
}

#if 1
std::string git_add_file(const fs::path& file_path, bool root){
    fs::path project_base_path = repo_find(file_path);
    fs::path git_path = project_base_path / ".cpp-git";
    GitTree tree_obj(git_path);

    GitTree* previous_commit_tree = getTreeObjectOfHEAD();
    for (auto node:previous_commit_tree->directory){
        if(node.name==file_path.filename()){
            std::cout << "File Path: " << file_path << std::endl;
            //create GitBlob object and write to .git
            std::string blob_hash = readFileAndWriteObject(file_path);
            // add to new_tree
            tree_obj.add_entry(node.type,node.name,blob_hash);
        }
        else{
            tree_obj.add_entry(node.type,node.name,node.hash);
        }
    }
    std::string output =  writeObject(&tree_obj);
    if (root){
        std::cout << "Should write to index now" << std::endl;
        write_file(git_path / "index",output);
    }
    return output;
}
#endif

// TODO: change git_add back to folder based + change name of function

// BC: For now, can only add from project root
// since initial git_add will start on project's root
// and add all blobs relative to it
std::string createIndexTreeFromFolder(const fs::path& adding_directory, bool root){
    fs::path project_base_path = repo_find(adding_directory);
    fs::path git_path = project_base_path / ".cpp-git";

    GitTree tree_obj(git_path);
    for (auto entry: fs::directory_iterator(adding_directory)){
        fs::path path = entry.path();
        std::cout << "Entry path: " << path << std::endl;
        if (fs::is_regular_file(entry)){
            std::string blob_hash = readFileAndWriteObject(path);
            // append to tree object
            tree_obj.add_entry("blob",path.filename(),blob_hash);
        }
        else if (fs::is_directory(path)){
            std::string tree_hash = createIndexTreeFromFolder(path);
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
