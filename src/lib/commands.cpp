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

Repo git_init(fs::path source_path){
    fs::path git_path = source_path;
    // check that .git doesn't exist or is empty directory
    git_path /= ".cpp-git";
    if (fs::exists(git_path)){
        throw "init error: Not an empty path";
    }
    // initialize a repo object
    Repo repo(source_path,true);

    //create object dir
    fs::create_directories(repo.gitdir / "objects");
    //create HEAD file with "ref: refs/heads/master"
    create_file(repo.gitdir / "heads" / "master","ref: refs/heads/master");
    //create branches dir
    fs::create_directories(repo.gitdir / "branches");
    //create refs dir with tags+heads subdirectory
    fs::create_directories(repo.gitdir / "refs" / "tags");
    fs::create_directories(repo.gitdir / "refs" / "heads");

    return repo;
}
