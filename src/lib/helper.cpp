#include "helper.hpp"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <vector>
#include <sha1.hpp>
#include <range/v3/algorithm.hpp>
#include <range/v3/range/conversion.hpp>
namespace fs = std::filesystem;

template<typename T>
void printer(T container){
    for(auto x:container){
        std::cout << x << " ";
    }
    std::cout << std::endl;
}

/* ********* Member Functions	********* */
/* Repo::Repo(fs::path source_path, bool force = false){ */
/*     worktree = source_path; */
/*     gitdir = worktree / ".cpp-git"; */

/*     if (!force && !fs::is_empty(gitdir)){ */
/*         throw "Repo class constructor error: Not an empty path"; */
/*     } */
/* } */

GitObject::GitObject(fs::path git_path) {
    this->git_path = git_path;
    // implicity change to string
}
GitCommit::GitCommit(fs::path git_path,const std::string& data):GitObject(git_path){
    to_internal(data);
}
GitTree::GitTree(fs::path git_path,const std::string& data):GitObject(git_path){
    to_internal(data);
}
GitTag::GitTag(fs::path git_path,const std::string& data):GitObject(git_path){
    to_internal(data);
}
GitBlob::GitBlob(fs::path git_path,const std::string& data):GitObject(git_path){
    to_internal(data);
}


void GitCommit::to_internal(const std::string& data){
    // extract tree_hash
    auto tree_hash_point = std::find_if(data.begin(),data.end(),[](auto x){return x=='\n';});
    if (tree_hash_point == data.end()){
        throw "sommething wrong with commit file";
    }
    std::copy(data.begin(),tree_hash_point,std::back_inserter(tree_hash));
    /* std::cout << "Tree hash length: " << tree_hash.length() << std::endl; */
    // extract parent hash
    tree_hash_point = tree_hash_point+1;
    auto parent_hash_point = std::find_if(tree_hash_point,data.end(),[](auto x){return x == '\n';});
    if (parent_hash_point == data.end()){
        throw "sommething wrong with commit file";
    }
    std::copy(tree_hash_point,parent_hash_point,std::back_inserter(parent_hash));
    /* std::cout << "Parent hash length: " << parent_hash.length() << std::endl; */
    // extract commit_message
    parent_hash_point = parent_hash_point+1;
    std::copy(parent_hash_point,data.end(),std::back_inserter(commit_message));
    /* std::cout << "Commit message length: " << commit_message.length() << std::endl; */
}

/* template<typename T> */
/* set_part(std::string &member, T range){ */
/*     std::transform() */
/* } */

std::ostream& operator<<(std::ostream& os, GitTreeNode& t){
    os << "Name: " << t.name << " Type: " << t.type << " Hash: " << t.hash << std::endl;
    return os;
}

void GitTree::to_internal(const std::string& data){
    using namespace ranges;
    std::vector<char> no_null(data.length()+1);
    std::copy(data.begin(),data.end(),no_null.begin());
    /* std::cout << "Before remove:" << std::endl; */
    /* printer(no_null); */
    no_null.pop_back();
    /* std::cout << "After remove:" << std::endl; */
    /* printer(no_null); */

    auto entry_list = no_null | views::split('\n');
    for(auto entry:entry_list){
        // extract the parts
        auto entry_parts = entry | views::split(' ');
        GitTreeNode tmp;
        for (auto [idx,part]: views::enumerate(entry_parts)){
            if (idx==0){
                /* set_part(tmp.name,part); */
                /* tmp.name = std::string(part.begin(),part.end()); */
                tmp.type = to<std::string>(part);
            }
            else if (idx==1){
                /* set_part(tmp.type,part); */
                /* tmp.type = std::string(part.begin(),part.end()); */
                tmp.name = to<std::string>(part);
            }
            else if (idx ==2){
                /* set_part(tmp.hash,part); */
                /* tmp.hash = std::string(part.begin(),part.end()); */
                tmp.hash = to<std::string>(part);
            }
            else{
                throw "should not have more than 3 parts";
            }
        }
        // update vector
        directory.push_back(tmp);
    }
    /* printer(directory); */
}
void GitTag::to_internal(const std::string& data){
    //TODO
}
void GitBlob::to_internal(const std::string& data){
    this-> data = data;
}


std::string GitCommit::to_filesystem(void){
    return tree_hash + '\n' + parent_hash + '\n' + commit_message;
}
std::string GitTree::to_filesystem(void){
    std::string data;
    for(auto node:directory){
        data += node.type + ' ' + node.name + ' ' + node.hash + '\n';
    }
    // to account for extra '\n'
    data.pop_back();
    return data;
}
std::string GitTag::to_filesystem(void){
    return "TODO";
}
std::string GitBlob::to_filesystem(void){
    return data;
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

GitObject* readObject(fs::path git_path, std::string hash) {
    fs::path object_path = git_path / "objects" / hash.substr(0, 2) / hash.substr(2);
    std::string content = read_file(object_path);
    // remove EOF
    content.pop_back();
    /* std::cout << "Read(File After EOF Remove): " << content.length() << std::endl; */

    auto split_index = content.find('\0');
    /* std::cout << "Split index: " << split_index << std::endl; */
    if (split_index == content.length() - 1) {
        throw "not a valid file";
    }
    // it's too bad string_view cannot be used in STL algorithms
    /* std::string_view type(content.begin(),split_point); */
    std::string type = content.substr(0, split_index);
    std::string data = content.substr(split_index + 1);
    /* std::cout << "Read in Data Length: " << data.length() << std::endl; */
    /* std::cout << data << std::endl; */

    return create_object(type, data, git_path);
}

std::string writeObject(GitObject* obj,bool write){
    std::string total = obj -> get_fmt() + '\0' + obj -> to_filesystem();
    /* std::cout << "Write(String): " << total.length() << std::endl; */
    /* std::cout << total << std::endl; */

    /* std::string hash = SHA1::from_file("test"); */
    SHA1 checksum;
    checksum.update(total);
    std::string hash = checksum.final();

    if (write){
        create_file(obj->git_path / "objects" / hash.substr(0,2) / hash.substr(2),total);
    }
    return hash;
}

