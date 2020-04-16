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
