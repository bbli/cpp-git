#include "helper.hpp"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <vector>
#include <sha1.hpp>
namespace fs = std::filesystem;

/* ********* Member Functions	********* */
/* Repo::Repo(fs::path source_path, bool force = false){ */
/*     worktree = source_path; */
/*     gitdir = worktree / ".cpp-git"; */

/*     if (!force && !fs::is_empty(gitdir)){ */
/*         throw "Repo class constructor error: Not an empty path"; */
/*     } */
/* } */

GitObject::GitObject(fs::path git_path, std::string& data) {
    this->git_path = git_path;
    // implicity change to string
    this->data = data;
}

std::string GitCommit::get_fmt(void){
    return "commit";
}

std::string GitTree::get_fmt(void){
    return "tree";
}

std::string GitTag::get_fmt(void){
    return "tag";
}

std::string GitBlob::get_fmt(void){
    return "blob";
}

/* ********* Functions	********* */
// Creates file and all parent directories if it does not exist
void create_file(fs::path file_path, std::string message) {
    auto dir_path = file_path.parent_path();
    fs::create_directories(dir_path);

    std::ofstream outfile(file_path.string());
    outfile << message << std::endl;
}

// get back to project root
fs::path repo_find(fs::path file_path) {
    if (file_path.parent_path()==file_path){
        throw "No git directory";
    }
    if (fs::exists(file_path / ".cpp-git")){
        return file_path;
    }
    else{
        return repo_find(file_path.parent_path());
    }
}

std::string read_file(fs::path path) {
    std::ifstream object_file;
    // Weakly typed, so should convert to string
    object_file.open(path);
    if (object_file.is_open()) {
        return std::string((std::istreambuf_iterator<char>(object_file)),
                           std::istreambuf_iterator<char>());
    } else {
        throw "Couldn't open the file";
    }
}

GitObject* create_object(std::string type, std::string& data, fs::path git_path) {
    if (type == std::string("commit")) return new GitCommit(git_path, data);
    if (type == std::string("tree")) return new GitTree(git_path, data);
    if (type == std::string("tag")) return new GitTag(git_path, data);
    if (type == std::string("blob")) return new GitBlob(git_path, data);
    throw "Something is wrong with the type";
}

GitObject* object_read(fs::path git_path, std::string hash) {
    fs::path object_path = git_path / "objects" / hash.substr(0, 2) / hash.substr(2);
    std::string content = read_file(object_path);

    // Assumes type is the first line of the hashed file
    /* auto split_point = std::find_if(content.begin(),content.end(),[](auto x){return x == '\n';});
     */
    auto split_index = content.find('\n');
    std::cout << "Split index: " << split_index << std::endl;
    if (split_index == content.length() - 1) {
        throw "not a valid file";
    }
    /* std::string_view type(content.begin(),split_point); */
    std::string type = content.substr(0, split_index);
    std::string data = content.substr(split_index + 1);

    return create_object(type, data, git_path);
}

std::string object_write(GitObject& obj,bool write){
    std::string total = obj.get_fmt() + '\n' + obj.data;
    std::cout << "Total: " << total << std::endl;
    std::string hash = SHA1::from_file(total);

    if (write){
        create_file(obj.git_path / "objects" / hash.substr(0,2) / hash.substr(2),total);
    }

    return hash;
}
