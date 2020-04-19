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

void git_init(fs::path project_base_path){
    // check that .git doesn't exist or is empty directory
    fs::path git_path = project_base_path / ".cpp-git";
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

std::string git_add_file_helper(std::string old_tree_hash, typename fs::path::iterator file_it,const typename fs::path::iterator end_it, const fs::path git_path, const fs::path& file_path){
    // New Tree Object we are creating
    GitTree tree_obj(git_path);
    // Old Tree Object we are iterating over
    GitTree* old_tree_obj = dynamic_cast<GitTree*>(readObject(git_path,old_tree_hash));
    std::cout << "Currently at: " << *file_it << std::endl;
    bool found = false;
    auto found_it = file_it;
    for (auto node:old_tree_obj->directory){
        // Case 1: Same branch as file
        if(node.name==*file_it){
            // SubCase 1: file_it is at last element
            auto check_it = file_it;
            if (++check_it == end_it){
                //create GitBlob object and write to .git
                std::string blob_hash = readFileAndWriteObject(git_path,file_path);
                // add to new_tree
                tree_obj.add_entry(node.type,node.name,blob_hash);
                found = true;
            }
            // SubCase 2 : file_it still refers to a folder
            else {
                file_it++;
                if (node.type != "tree"){
                    throw "this isn't a tree";
                }
                std::string tree_hash = git_add_file_helper(node.hash,file_it,end_it,git_path,file_path);
                // Q: how do I know that this is guaranteed to be a tree?
                tree_obj.add_entry("tree",node.name,tree_hash);
            }
        }
        // Case 2: Not what we want to add to index
        else{
            tree_obj.add_entry(node.type,node.name,node.hash);
        }
    }
    // EC: if at end and we haven't found the file in the old tree directory
    if (++found_it== end_it && !found){
        //create GitBlob object and write to .git
        std::string blob_hash = readFileAndWriteObject(git_path,file_path);
        // add to new_tree
        tree_obj.add_entry("blob",file_path.filename(),blob_hash);
    }
    //Write Tree and return hash
    return writeObject(&tree_obj);
}

#if 1
// NOTE: can add new files, but they must be in existing folders already
std::string git_add_file(const fs::path& file_path){
    fs::path project_base_path = repo_find(file_path);
    fs::path git_path = project_base_path / ".cpp-git";
    GitTree tree_obj(git_path);
    
    // Run file_it
    auto base_it = project_base_path.begin();
    auto file_it = file_path.begin();
    while(base_it != project_base_path.end()){
        base_it++;
        file_it++;
    }
    // *file_it=folder in /project_base_path/folder/etc...
    // Paths are unique, but individual names arn't
    // EC: if file is one_level

    std::string old_tree_hash = getTreeHashOfHead(git_path);
    std::cout << "HashOfHead:" << old_tree_hash << std::endl;
    std::string new_tree_hash = git_add_file_helper(old_tree_hash,file_it,file_path.end(),git_path,file_path);
    // EC: if file is new file

    std::cout << "Should write to index now" << std::endl;
    write_file(git_path / "index",new_tree_hash);
    return new_tree_hash;
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
        if (isGitRepo(path)){
            continue;
        }
        std::cout << "Entry path: " << path << std::endl;
        if (fs::is_regular_file(entry)){
            std::string blob_hash = readFileAndWriteObject(git_path,path);
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
