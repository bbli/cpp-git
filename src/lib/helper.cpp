#include "helper.hpp"
#include "git_objects.hpp"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
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

void printTree(fs::path git_path, std::string tree_hash) {
    GitTree* tree_obj = dynamic_cast<GitTree*>(readObject(git_path, tree_hash));
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

/* ********* Moving between project and git folders	********* */
std::string readProjectFileAndWriteObject(const fs::path git_path, const fs::path& file_path) {
    // read in and make GitBlob
    std::string content = read_file(file_path);
    GitBlob blob_obj = GitBlob(git_path, content);
    // write to object folder
    return writeObject(&blob_obj);
}

void writeObjectToProjectFile(fs::path project_blob_path, std::string blob_hash) {
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
            writeObjectToProjectFile(tree_write_path / node.name, node.hash);
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


/* ********* Traversal Helpers	********* */
// will only be direct if chkout a commit without a ref
// NOTE: instead of referencing a reference, 
// just have HEAD point to a commit object instead
void dereference_if_indirect(std::string& commit_string) {
    auto idx = commit_string.find(' ');
    if (idx != std::string::npos) {
        commit_string.erase(0, idx + 1);
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

bool isGitRepo(const fs::path& path) {
    std::string name = path.filename();
    if (name == ".cpp-git") {
        return true;
    } else {
        return false;
    }
}
bool checkNodeName(GitTreeNode& node, std::string file_it_name){
    return node.name == file_it_name;
}
bool endOfPath(typename fs::path::iterator file_it, typename fs::path::iterator end_it){
    auto check_it = file_it;
    return ++check_it == end_it;
}

void checkIfTree(GitTreeNode& node){
    if (node.type != "tree") {
        throw "this isn't a tree";
    }
}

GitTree* get_tree(std::string hash, fs::path git_path){
    GitObject* obj = readObject(git_path,hash);
    return dynamic_cast<GitTree*>(obj);
}

/* ********* Traversal Algorithms	********* */
std::string readProjectFolderAndWriteTree(const fs::path& adding_directory, bool index) {
    fs::path project_base_path = repo_find(adding_directory);
    fs::path git_path = project_base_path / ".cpp-git";

    GitTree tree_obj(git_path);
    for (auto entry : fs::directory_iterator(adding_directory)) {
        fs::path path = entry.path();
        if (isGitRepo(path)) {
            continue;
        }
        std::cout << "Entry path: " << path << std::endl;
        if (fs::is_regular_file(entry)) {
            std::string blob_hash = readProjectFileAndWriteObject(git_path, path);
            // append to tree object
            tree_obj.add_entry("blob", path.filename(), blob_hash);
        } else if (fs::is_directory(path)) {
            std::string tree_hash = readProjectFolderAndWriteTree(path);
            tree_obj.add_entry("tree", path.filename(), tree_hash);
        } else {
            throw "cpp-git cannot handle this file";
        }
    }
    std::string output = writeObject(&tree_obj);
    if (index) {
        std::cout << "Should write to index now" << std::endl;
        write_file(git_path / "index", output);
    }
    return output;
}

std::string getSubTreeHashForNewFile(GitTree* tree_obj, typename fs::path::iterator file_it,
                                const typename fs::path::iterator end_it, const fs::path git_path,
                                const fs::path& file_path) {
    // New Tree Object we are creating
    GitTree new_tree_obj(git_path);

    /* std::cout << "Currently at: " << *file_it << std::endl; */
    bool found = false;
    for (auto node : tree_obj->directory) {
        // Case 1: Same branch as file
        if (checkNodeName(node,*file_it)) {
            // SubCase 1: file_it is at last element
            bool end = endOfPath(file_it,end_it);
            if (end) {
                // create GitBlob object, write to .git, then add to tree obj
                std::string blob_hash = readProjectFileAndWriteObject(git_path, file_path);
                new_tree_obj.add_entry(node.type, node.name, blob_hash);
                found = true;
            }
            // SubCase 2 : still haven't reached file
            else {
                checkIfTree(node);
                GitTree* subtree = get_tree(node.hash,git_path);
                auto new_it = file_it;
                std::string subtree_hash =
                    getSubTreeHashForNewFile(subtree, ++new_it, end_it, git_path, file_path);
                new_tree_obj.add_entry("tree", node.name, subtree_hash);
            }
        }
        // Case 2: Not what we want to add to index
        else {
            new_tree_obj.add_entry(node.type, node.name, node.hash);
        }
    }
    // EC: if at end and we haven't found the file in the old tree directory
    std::cout << "Currently at: " << *file_it << std::endl;
    if (endOfPath(file_it,end_it) && !found) {
        std::string blob_hash = readProjectFileAndWriteObject(git_path, file_path);
        new_tree_obj.add_entry("blob", file_path.filename(), blob_hash);
    }
    // Write Tree and return hash
    std::cout << "Post Listing: " << std::endl;
    printer(new_tree_obj.directory);
    return writeObject(&new_tree_obj);
}
std::string getSubTreeHashForNewFolder(GitTree* tree_obj, typename fs::path::iterator file_it,
                                  typename fs::path::iterator end_it, const fs::path git_path,
                                  const fs::path folder_path) {
    // New Tree Object we are creating
    GitTree new_tree_obj(git_path);

    std::cout << "Pre: currently at " << *file_it << std::endl;
    bool found = false;
    for (auto node : tree_obj->directory) {
        // Case 1: Same branch as file
        if (checkNodeName(node,*file_it)) {
            // SubCase 1: node points to adding folder
            bool end = endOfPath(file_it,end_it);
            if (end) {
                checkIfTree(node);
                std::string subtree_hash = readProjectFolderAndWriteTree(folder_path);
                // add to new_tree
                new_tree_obj.add_entry(node.type, node.name, subtree_hash);
                found = true;
            }
            // SubCase 2 : still haven't reached folder
            else {
                checkIfTree(node);
                GitTree* subtree = get_tree(node.hash,git_path);
                auto new_it = file_it;
                std::string subtree_hash =
                    getSubTreeHashForNewFolder(subtree, ++new_it, end_it, git_path, folder_path);
                new_tree_obj.add_entry("tree", node.name, subtree_hash);
            }
        }
        // Case 2: Not what we want to add to index
        else {
            new_tree_obj.add_entry(node.type, node.name, node.hash);
        }
    }
    // EC: if at end and we haven't found the file in the old tree directory
    if (endOfPath(file_it,end_it) && !found) {
        std::string subtree_hash = readProjectFolderAndWriteTree(folder_path, false);
        // add to new_tree
        new_tree_obj.add_entry("blob", folder_path.filename(), subtree_hash);
    }
    // Write Tree and return hash
    return writeObject(&new_tree_obj);
}

/* ********* Finding Algorithms	********* */
std::string findHashInTree(std::string tree_hash, typename fs::path::iterator file_it,
                             const typename fs::path::iterator end_it, const fs::path git_path) {
    GitTree* tree_obj = dynamic_cast<GitTree*>(readObject(git_path, tree_hash));
    for (auto node : tree_obj->directory) {
        if (checkNodeName(node,*file_it)) {
            bool end = endOfPath(file_it,end_it);
            if (end) {
                return node.hash;
            }
            // continue down
            else {
                file_it++;
                checkIfTree(node);
                return findHashInTree(node.hash, file_it, end_it, git_path);
            }
        }
    }
    throw "Couldn't find file in this tree";
}

GitBlob* findProjectFileFromTree(std::string tree_hash, fs::path relative_path, const fs::path git_path) {
    auto path_it = relative_path.begin();
    auto end_it = relative_path.end();
    std::string file_hash = findHashInTree(tree_hash, path_it, end_it, git_path);
    GitObject* obj = readObject(git_path, file_hash);
    return dynamic_cast<GitBlob*>(obj);
}

GitTree* findProjectFolderFromTree(std::string tree_hash, fs::path relative_path,
                            const fs::path git_path) {
    auto path_it = relative_path.begin();
    auto end_it = relative_path.end();
    std::cout << "Starting Folder Find" << std::endl;
    std::string file_hash = findHashInTree(tree_hash, path_it, end_it, git_path);
    GitObject* obj = readObject(git_path, file_hash);
    return dynamic_cast<GitTree*>(obj);
}

/* ********* Commit Helpers	********* */
GitTree* getIndexTree(fs::path git_path) {
    std::string tree_hash = read_file(git_path / "index");
    GitObject* obj = readObject(git_path, tree_hash);
    return dynamic_cast<GitTree*>(obj);
}

// NOTE: until commit objects are implemented, will just write tree hashes to HEAD
std::string get_head_tree(fs::path git_path) {
    std::string commit_hash = read_file(git_path / "HEAD");
    /* dereference_if_indirect(content); */
    /* GitObject* obj = readObject(project_base_path / ".cpp-git", content); */
    /* return dynamic_cast<GitTree*>(obj); */
    return commit_hash;
}
