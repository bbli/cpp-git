#include <vector>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <algorithm>
#include <iostream>

#include "helper.hpp"
namespace fs = std::filesystem;

#if 1
/* ********* Member Functions	********* */
/* Repo::Repo(fs::path source_path, bool force = false){ */
/*     worktree = source_path; */
/*     gitdir = worktree / ".cpp-git"; */

/*     if (!force && !fs::is_empty(gitdir)){ */
/*         throw "Repo class constructor error: Not an empty path"; */
/*     } */
/* } */

GitObject::GitObject(fs::path git_path,std::string data){
    this->git_path = git_path;
    //implicity change to string
    this->data = data;
}

/* ********* Functions	********* */
//Creates file and all parent directories if it does not exist
void create_file(fs::path file_path,std::string message){ 
    auto dir_path = file_path.parent_path();
    fs::create_directories(dir_path);

    std::ofstream outfile(file_path.string());
    outfile << message << std::endl;
}

fs::path repo_find(fs::path file_path,bool required = true){
    // get back to project root
}

std::string read_file(fs::path path){
    std::ifstream object_file;
    // Weakly typed, so should convert to string
    object_file.open(path);
    if (object_file.is_open()){
        return std::string((std::istreambuf_iterator<char>(object_file)),std::istreambuf_iterator<char>());
    }
    else{
        throw "Couldn't open the file";
    }
}

GitObject create_object(std::string type, std::string data,fs::path git_path){
    if (type== std::string("commit"))
            return GitCommit(git_path,data);
    if (type== std::string("tree"))
            return GitTree(git_path,data);
    if (type== std::string("tag"))
            return GitTag(git_path,data);
    if (type== std::string("blob"))
            return GitBlob(git_path,data);
    throw "Something is wrong with the type";
}

GitObject object_read(fs::path git_path, std::string hash){
    fs::path object_path = git_path / "objects" / hash.substr(0,2) /hash.substr(2);
    std::string content = read_file(object_path);

    // Assumes type is the first line of the hashed file
    /* auto split_point = std::find_if(content.begin(),content.end(),[](auto x){return x == '\n';}); */
    auto split_index = content.find('\n');
    if (split_index ==content.length()-1){
        throw "not a valid file";
    }
    /* std::string type(content.begin(),split_point); */
    std::string type = content.substr(0,split_index);
    std::string data = content.substr(split_index+1);

    return create_object(type,data,git_path);
}


#endif
