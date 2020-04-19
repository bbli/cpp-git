#include "helper.hpp"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <range/v3/algorithm.hpp>
#include <range/v3/range/conversion.hpp>
#include <sha1.hpp>
#include <vector>
namespace fs = std::filesystem;

template <typename T>
void printer(T container) {
    for (auto x : container) {
        std::cout << x << " ";
    }
    std::cout << std::endl;
}

void printTree(fs::path git_path, std::string tree_hash){
    GitTree* tree_obj = dynamic_cast<GitTree*>(readObject(git_path,tree_hash));
    printer(tree_obj->directory);
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
GitCommit::GitCommit(fs::path git_path, const std::string& data) : GitObject(git_path) {
    to_internal(data);
}
GitTree::GitTree(fs::path git_path, const std::string& data) : GitObject(git_path) {
    to_internal(data);
}
GitTree::GitTree(fs::path git_path) : GitObject(git_path){};
GitTag::GitTag(fs::path git_path, const std::string& data) : GitObject(git_path) {
    to_internal(data);
}
GitBlob::GitBlob(fs::path git_path, const std::string& data) : GitObject(git_path) {
    to_internal(data);
}

void GitCommit::to_internal(const std::string& data) {
    // extract tree_hash
    auto tree_hash_point = std::find_if(data.begin(), data.end(), [](auto x) { return x == '\n'; });
    if (tree_hash_point == data.end()) {
        throw "sommething wrong with commit file";
    }
    std::copy(data.begin(), tree_hash_point, std::back_inserter(tree_hash));
    /* std::cout << "Tree hash length: " << tree_hash.length() << std::endl; */
    // extract parent hash
    tree_hash_point = tree_hash_point + 1;
    auto parent_hash_point =
        std::find_if(tree_hash_point, data.end(), [](auto x) { return x == '\n'; });
    if (parent_hash_point == data.end()) {
        throw "sommething wrong with commit file";
    }
    std::copy(tree_hash_point, parent_hash_point, std::back_inserter(parent_hash));
    /* std::cout << "Parent hash length: " << parent_hash.length() << std::endl; */
    // extract commit_message
    parent_hash_point = parent_hash_point + 1;
    std::copy(parent_hash_point, data.end(), std::back_inserter(commit_message));
    /* std::cout << "Commit message length: " << commit_message.length() << std::endl; */
}

/* template<typename T> */
/* set_part(std::string &member, T range){ */
/*     std::transform() */
/* } */

std::ostream& operator<<(std::ostream& os, GitTreeNode& t) {
    os << "Name: " << t.name << " Type: " << t.type << " Hash: " << t.hash << std::endl;
    return os;
}

void GitTree::to_internal(const std::string& data) {
    using namespace ranges;
    std::vector<char> no_null(data.length() + 1);
    std::copy(data.begin(), data.end(), no_null.begin());
    /* std::cout << "Before remove:" << std::endl; */
    /* printer(no_null); */
    no_null.pop_back();
    /* std::cout << "After remove:" << std::endl; */
    /* printer(no_null); */

    auto entry_list = no_null | views::split('\n');
    for (auto entry : entry_list) {
        // extract the parts
        auto entry_parts = entry | views::split(' ');
        GitTreeNode tmp;
        for (auto [idx, part] : views::enumerate(entry_parts)) {
            if (idx == 0) {
                /* set_part(tmp.name,part); */
                /* tmp.name = std::string(part.begin(),part.end()); */
                tmp.type = to<std::string>(part);
            } else if (idx == 1) {
                /* set_part(tmp.type,part); */
                /* tmp.type = std::string(part.begin(),part.end()); */
                tmp.name = to<std::string>(part);
            } else if (idx == 2) {
                /* set_part(tmp.hash,part); */
                /* tmp.hash = std::string(part.begin(),part.end()); */
                tmp.hash = to<std::string>(part);
            } else {
                throw "should not have more than 3 parts";
            }
        }
        // update vector
        directory.push_back(tmp);
    }
    /* printer(directory); */
}
void GitTag::to_internal(const std::string& data) {
    // TODO
}
void GitBlob::to_internal(const std::string& data) { this->data = data; }

std::string GitCommit::to_filesystem(void) {
    return tree_hash + '\n' + parent_hash + '\n' + commit_message;
}
std::string GitTree::to_filesystem(void) {
    std::string data;
    for (auto node : directory) {
        data += node.type + ' ' + node.name + ' ' + node.hash + '\n';
    }
    // to account for extra '\n'
    data.pop_back();
    return data;
}
std::string GitTag::to_filesystem(void) { return "TODO"; }
std::string GitBlob::to_filesystem(void) { return data; }

std::string GitCommit::get_fmt(void) { return "commit"; }

std::string GitTree::get_fmt(void) { return "tree"; }

std::string GitTag::get_fmt(void) { return "tag"; }

std::string GitBlob::get_fmt(void) { return "blob"; }

/* ********* Functions	********* */
void GitTree::add_entry(std::string type, std::string file_name, std::string hash) {
    if (type == "blob" || type == "tree"){
        directory.push_back({type, file_name, hash});
    } else {
        throw "Type should only be blob or tree";
    }
}
// Creates file and all parent directories if it does not exist
void write_file(fs::path file_path, std::string message) {
    auto dir_path = file_path.parent_path();
    fs::create_directories(dir_path);

    std::ofstream outfile(file_path.string());
    outfile << message << std::endl;
}
std::string read_file(fs::path path) {
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

GitObject* create_object(std::string type, std::string& data, fs::path git_path) {
    if (type == std::string("commit")) return new GitCommit(git_path, data);
    if (type == std::string("tree")) return new GitTree(git_path, data);
    if (type == std::string("tag")) return new GitTag(git_path, data);
    if (type == std::string("blob")) return new GitBlob(git_path, data);
    throw "Something is wrong with the type";
}

GitObject* readObject(fs::path git_path, std::string hash) {
    fs::path object_path = git_path / "objects" / hash.substr(0, 2) / hash.substr(2);
    /* std::cout << "object_path: " << object_path << std::endl; */
    std::string content = read_file(object_path);
    // remove EOF
    /* content.pop_back(); */
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
    /* std::cout << "Data: " << data << std::endl; */

    return create_object(type, data, git_path);
}

std::string writeObject(GitObject* obj, bool write) {
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

void blob_to_file(fs::path project_blob_path, std::string blob_hash) {
    fs::path git_path = repo_find(project_blob_path) / ".cpp-git";
    GitObject* obj = readObject(git_path, blob_hash);

    GitBlob* blob_obj = dynamic_cast<GitBlob*>(obj);
    write_file(project_blob_path, blob_obj->data);
}

void walkTreeAndReplace(fs::path tree_write_path, GitObject* obj) {
    fs::path git_path = repo_find(tree_write_path) / ".cpp-git";
    
    GitTree* tree_obj = dynamic_cast<GitTree*>(obj);
    std::cout << "Tree listing: " << std::endl;
    printer(tree_obj->directory);
    for (auto node : tree_obj->directory) {
        if (node.type == "blob") {
            /* std::cout << "Write Path: "<< (tree_write_path / node.name) << std::endl; */
            blob_to_file(tree_write_path / node.name, node.hash);
        } else if (node.type == "tree") {
            /* std::cout << "Got here" << std::endl; */
            GitObject* obj = readObject(git_path, node.hash);
            walkTreeAndReplace(tree_write_path / node.name, obj);
        } else {
            std::cout << "Node should only be tree or blob" << std::endl;
            throw "Node type should only be tree or blob";
        }
    }
}

void howToGetAbsolutePath(fs::path git_path, GitObject* obj) {
    // HMM???
}

void chkout_obj(fs::path git_path, std::string hash) {
    GitObject* obj = readObject(git_path, hash);
    std::string fmt = obj->get_fmt();
    if (fmt == "commit") {
        GitCommit* commit_obj = dynamic_cast<GitCommit*>(obj);
        chkout_obj(git_path, commit_obj->tree_hash);
    } else if (fmt == "tree") {
        fs::path repo_base_path = repo_find(fs::current_path());
        walkTreeAndReplace(repo_base_path, obj);
    } else if (fmt == "blob") {
        howToGetAbsolutePath(git_path, obj);
    } else {
        throw "Shouldn't reach here";
    }
}

void dereference_if_indirect(std::string& commit_string){
    auto idx = commit_string.find(' ');
    if (idx != std::string::npos){
        commit_string.erase(0,idx+1);
    }
}

std::string getTreeHashOfHead(fs::path git_path){
    std::string content = read_file(git_path/ "HEAD");
    dereference_if_indirect(content);
    /* GitObject* obj = readObject(project_base_path / ".cpp-git", content); */
    /* return dynamic_cast<GitTree*>(obj); */
    return content;
}

std::string readFileAndWriteObject(const fs::path git_path, const fs::path& file_path){
    // read in and make GitBlob
    std::string content = read_file(file_path);
    GitBlob blob_obj = GitBlob(git_path,content);
    // write to object folder
    return writeObject(&blob_obj);
}

bool isGitRepo(const fs::path& path){
    std::string name = path.filename();
    if (name==".cpp-git"){
        return true;
    }
    else{
        return false;
    }
}
#if 1
std::string find_FileFromTree_helper(std::string tree_hash,typename fs::path::iterator file_it, const typename fs::path::iterator end_it, const fs::path git_path){
    GitTree* tree_obj = dynamic_cast<GitTree*>(readObject(git_path,tree_hash));
    for(auto node: tree_obj->directory){
        if (node.name == *file_it){
            /* std::cout << "Node name: " << node.name << std::endl; */
            auto check_it = file_it;
            //reached end, so this should be the file
            if(++check_it == end_it){
                if (node.type != "blob"){
                    throw "this isn't a blob";
                }
                return node.hash;
            }
            // continue down
            else{
                file_it++;
                if (node.type != "tree"){
                    throw "this isn't a tree";
                }
                return find_FileFromTree_helper(node.hash,file_it,end_it,git_path);
            }
        }
    }
    throw "Couldn't find file in this tree";
}
GitBlob* findFileFromTree(std::string tree_hash, fs::path relative_path, const fs::path git_path){
    auto path_it = relative_path.begin();
    auto end_it = relative_path.end();
    std::string file_hash = find_FileFromTree_helper(tree_hash, path_it,end_it,git_path);
    GitObject* obj = readObject(git_path,file_hash);
    return dynamic_cast<GitBlob*>(obj);
}
#endif

