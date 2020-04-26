#include "helper.hpp"
#include "git_objects.hpp"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sha1.hpp>
#include <vector>
#include <unordered_map>
namespace fs = std::filesystem;


void print_tree(fs::path git_path, std::string tree_hash) {
    GitTree* tree_obj = dynamic_cast<GitTree*>(read_object(git_path, tree_hash));
    printer(tree_obj->directory);
}

/* ********* Lower Level Functions	********* */
// Creates file and all parent directories if it does not exist
void write_file(fs::path file_path, std::string message) {
    auto dir_path = file_path.parent_path();
    fs::create_directories(dir_path);

    std::ofstream outfile(file_path.string());
    outfile << message << std::endl;
}

std::string read_file(fs::path path) {
    /* std::cout << "Reading: " << path << std::endl; */
    std::ifstream object_file;
    // Weakly typed, so should convert to string
    object_file.open(path);
    if (object_file.is_open()) {
        std::string tmp = std::string((std::istreambuf_iterator<char>(object_file)),
                                      std::istreambuf_iterator<char>());
        // account for extra \n or EOF char
        tmp.pop_back();
        return tmp;
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

GitObject* read_object(fs::path git_path, std::string hash) {
    /* git_path: worktree/.cpp-git; hash: hash of the object*/
    fs::path object_path = git_path / "objects" / hash.substr(0, 2) / hash.substr(2);
    /* std::cout << "object_path: " << object_path << std::endl; */
    std::string content = read_file(object_path);
    // remove EOF
    /* content.pop_back(); */
    /* std::cout << "Read(File After EOF Remove): " << content.length() << std::endl; */
    auto split_index = content.find('\0');
    /* std::cout << "Split index: " << split_index << std::endl; */
    // if (split_index == content.length() - 1) {
    //     std::cout<<"FFF"<<std::endl;
    //     //throw "not a valid file";
    // }
    // it's too bad string_view cannot be used in STL algorithms
    /* std::string_view type(content.begin(),split_point); */
    std::string type = content.substr(0, split_index);
    std::string data = content.substr(split_index + 1);
    /* std::cout << "Read in Data Length: " << data.length() << std::endl; */
    /* std::cout << "Data: " << data << std::endl; */
    return create_object(type, data, git_path);
}

std::string write_object(GitObject* obj, bool write) {
    std::string total = obj->get_fmt() + '\0' + obj->to_filesystem();
    /* std::cout << "Write(String): " << total.length() << std::endl; */
    /* std::cout << total << std::endl; */

    /* std::string hash = SHA1::from_file("test"); */
    SHA1 checksum;
    checksum.update(total);
    std::string hash = checksum.final();
    /* std::cout << "Hash: " << hash << std::endl; */

    if (write) {
        write_file(obj->git_path / "objects" / hash.substr(0, 2) / hash.substr(2), total);
    }
    return hash;
}

std::string object_find(fs::path repo, fs::path obj, const std::string& fmt) {
    return obj;
}

/* ********* Moving between project and git folders	********* */
std::string read_project_file_and_write_object(const fs::path git_path, const fs::path& file_path) {
    // read in and make GitBlob
    std::string content = read_file(file_path);
    GitBlob blob_obj = GitBlob(git_path, content);
    // write to object folder
    return write_object(&blob_obj);
}

void write_object_to_project_file(fs::path project_blob_path, std::string blob_hash) {
    fs::path git_path = repo_find(project_blob_path) / ".cpp-git";
    GitObject* obj = read_object(git_path, blob_hash);

    GitBlob* blob_obj = dynamic_cast<GitBlob*>(obj);
    write_file(project_blob_path, blob_obj->data);
}

std::string read_project_folder_and_write_tree(const fs::path& adding_directory, bool index) {
    fs::path project_base_path = repo_find(adding_directory);
    fs::path git_path = project_base_path / ".cpp-git";

    GitTree tree_obj(git_path);
    for (auto entry : fs::directory_iterator(adding_directory)) {
        fs::path path = entry.path();
        if (is_git_repo(path)) {
            continue;
        }
        /* std::cout << "Entry path: " << path << std::endl; */
        if (fs::is_regular_file(entry)) {
            std::string blob_hash = read_project_file_and_write_object(git_path, path);
            tree_obj.add_entry("blob", path.filename(), blob_hash);
        } else if (fs::is_directory(path)) {
            std::string tree_hash = read_project_folder_and_write_tree(path);
            tree_obj.add_entry("tree", path.filename(), tree_hash);
        } else {
            throw "cpp-git cannot handle this file";
        }
    }
    std::string output = write_object(&tree_obj);
    if (index) {
        std::cout << "Should write to index now" << std::endl;
        write_file(git_path / "index", output);
    }
    return output;
}

/* ********* Traversal Helpers	********* */

// get back to project root
fs::path repo_find(fs::path file_path) {
    if (file_path.parent_path() == file_path) {
        throw "No git directory";
    }
    if (fs::exists(file_path / ".cpp-git")) {
        return file_path;
    } else {
        return repo_find(file_path.parent_path());
    }
}

bool is_git_repo(const fs::path& path) {
    std::string name = path.filename();
    if (name == ".cpp-git") {
        return true;
    } else {
        return false;
    }
}
bool check_node_name(GitTreeNode& node, std::string file_it_name){
    return node.name == file_it_name;
}
bool end_of_path(typename fs::path::iterator file_it, typename fs::path::iterator end_it){
    auto check_it = file_it;
    return ++check_it == end_it;
}

void check_if_tree(GitTreeNode& node){
    if (node.type != "tree") {
        throw "this isn't a tree";
    }
}

GitTree* get_tree_from_hash(std::string hash, fs::path git_path){
    GitObject* obj = read_object(git_path,hash);
    return dynamic_cast<GitTree*>(obj);
}

std::string path_relative_to_project(const fs::path project_base_path,fs::path entry_path){
    std::string base = project_base_path.string();
    std::string entry = entry_path.string();

    auto [rel,_] = std::mismatch(entry.begin(),entry.end(),base.begin());
    auto idx = rel-entry.begin();

    return entry.substr(idx+1);
}

bool is_in_set(const std::set<std::string>& set,std::string val){
    auto it = set.find(val);
    if (it != set.end()){
        return true;
    }
    else{
        return false;
    }
}


/* ********* Finding Project File in GitTree	********* */
std::string find_hash_in_tree(std::string tree_hash, typename fs::path::iterator file_it,
                             const typename fs::path::iterator end_it, const fs::path git_path) {
    GitTree* tree_obj = dynamic_cast<GitTree*>(read_object(git_path, tree_hash));
    for (auto node : tree_obj->directory) {
        if (check_node_name(node,*file_it)) {
            bool end = end_of_path(file_it,end_it);
            if (end) {
                return node.hash;
            }
            // continue down
            else {
                file_it++;
                check_if_tree(node);
                return find_hash_in_tree(node.hash, file_it, end_it, git_path);
            }
        }
    }
    throw "Couldn't find file in this tree";
}

GitBlob* find_project_file_from_tree(std::string tree_hash, fs::path relative_path, const fs::path git_path) {
    auto path_it = relative_path.begin();
    auto end_it = relative_path.end();
    std::string file_hash = find_hash_in_tree(tree_hash, path_it, end_it, git_path);
    GitObject* obj = read_object(git_path, file_hash);
    return dynamic_cast<GitBlob*>(obj);
}

GitTree* find_project_folder_from_tree(std::string tree_hash, fs::path relative_path,
                            const fs::path git_path) {
    auto path_it = relative_path.begin();
    auto end_it = relative_path.end();
    std::cout << "Starting Folder Find" << std::endl;
    std::string file_hash = find_hash_in_tree(tree_hash, path_it, end_it, git_path);
    GitObject* obj = read_object(git_path, file_hash);
    return dynamic_cast<GitTree*>(obj);
}


/* ********* Commit Helpers	********* */
// will only be direct if chkout a commit without a ref
// NOTE: instead of referencing a reference, 
// just have HEAD point to a commit object instead
std::string get_commit_hash_from_branch(std::string full_branch_name, fs::path git_path){
    return read_file(git_path / full_branch_name);
}

// will return refs/heads/branch_name
// or commit hash
std::string get_current_branch(fs::path git_path){
    std::string content = read_file(git_path / "HEAD");
    auto idx = content.find(' ');
    if (idx != std::string::npos) {
        content.erase(0, idx + 1);
    }
    return content;
}

GitTree* get_index_tree(fs::path git_path) {
    std::string tree_hash = read_file(git_path / "index");
    if (tree_hash==""){
        return nullptr;
    }
    else{
        GitObject* obj = read_object(git_path, tree_hash);
        return dynamic_cast<GitTree*>(obj);
    }
}

// NOTE: until commit objects are implemented, will just write tree hashes to refs
GitCommit* get_commit_from_hash(std::string commit_hash, fs::path git_path){
        GitObject* obj = read_object(git_path,commit_hash);
        return dynamic_cast<GitCommit*>(obj);
}

GitTree* get_head_tree(fs::path git_path) {
    std::string full_branch_name = get_current_branch(git_path);
    std::string commit_hash = get_commit_hash_from_branch(full_branch_name,git_path);
    if (commit_hash==""){
        return nullptr;
    }
    else{
        /* std::cout << "Commit Hash: " << tree_hash << std::endl; */
        GitCommit* commit_obj = get_commit_from_hash(commit_hash,git_path);
        std::cout << "Commit tree: " << (commit_obj->tree_hash) << std::endl;
        return get_tree_from_hash(commit_obj->tree_hash,git_path);
    }
}
std::string get_tree_hash_of_index(fs::path git_path) {
    std::string content = read_file(git_path / "index");
    /* dereference_if_indirect(content); */
    /* GitObject* obj = read_object(project_base_path / ".cpp-git", content); */
    /* return dynamic_cast<GitTree*>(obj); */
    return content;
}
