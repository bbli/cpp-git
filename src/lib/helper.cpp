#include <vector>
#include <iostream>
#include <filesystem>
#include <fstream>

#include "helper.hpp"
namespace fs = std::filesystem;

#if 1
/* ********* Member Functions	********* */
Repo::Repo(fs::path source_path, bool force = false){
    worktree = source_path;
    gitdir = worktree / ".cpp-git";

    if (!force && !fs::is_empty(gitdir)){
        throw "Repo class constructor error: Not an empty path";
    }
}

/* ********* Functions	********* */
//Creates file and all parent directories if it does not exist
void create_file(fs::path file_path,std::string message){ 
    auto dir_path = file_path.parent_path();
    fs::create_directories(dir_path);
    std::ofstream outfile(file_path.string());
    outfile << message << std::endl;
    outfile.close();
}

fs::path repo_find(fs::path file_path,bool required = true){
    // get back to project root
}


#endif
