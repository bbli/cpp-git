#include <vector>
#include <iostream>
#include "helper.hpp"
#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

int test_function(void){
    std::vector<int> test;
    std::cout << "Hello World" << std::endl;
    return 0;
}
#if 1
/* ********* Helper Functions	********* */
//Creates file and all parent directories if it does not exist
void create_file(fs::path file_path,std::string message){ 
    auto dir_path = file_path.parent_path();
    fs::create_directories(dir_path);
    std::ofstream outfile(file_path.string());
    outfile << message << std::endl;
    outfile.close();
}

//if force=true -> empty git directory
Repo::Repo(std::string source_path, bool force = false){
    worktree = fs::path(source_path);
    gitdir = worktree / ".cpp-git";

    if (!force && !fs::is_empty(gitdir)){
        throw "Repo class constructor error: Not an empty path";
    }
}

/* ********* Main Functions	********* */
Repo git_init(std::string source_path){

    // check that .git doesn't exist or is empty directory
    fs::path git_path(source_path);
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

#endif
