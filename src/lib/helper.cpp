#include "helper.hpp"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <sha1.hpp>
#include <unordered_map>
#include <vector>

#include "git_objects.hpp"
namespace fs = std::filesystem;
using namespace std;

void print_tree(fs::path git_path, string tree_hash) {
    GitTree tree_obj;
    read_into_object(tree_obj, git_path, tree_hash);
    printer(tree_obj.directory);
}

/* ********* Lower Level Functions	********* */
// Creates file and all parent directories if it does not exist
void write_file(fs::path file_path, string message) {
    auto dir_path = file_path.parent_path();
    fs::create_directories(dir_path);

    ofstream outfile(file_path.string());
    outfile << message << endl;
}

string read_file(fs::path path) {
    /* cout << "Reading: " << path << endl; */
    ifstream object_file;
    // Weakly typed, so should convert to string
    object_file.open(path);
    if (object_file.is_open()) {
        string tmp = string((istreambuf_iterator<char>(object_file)), istreambuf_iterator<char>());
        // account for extra \n or EOF char
        tmp.pop_back();
        return tmp;
    } else {
        throw std::runtime_error("Couldn't open the file");
    }
}

GitObject* create_object(string type, string& data, fs::path git_path) {
    if (type == string("commit")) return new GitCommit(git_path, data);
    if (type == string("tree")) return new GitTree(git_path, data);
    if (type == string("tag")) return new GitTag(git_path, data);
    if (type == string("blob")) return new GitBlob(git_path, data);
    throw std::runtime_error("Something is wrong with the type");
}

GitObject* read_object(fs::path git_path, string hash) {
    /* git_path: worktree/.cpp-git; hash: hash of the object*/
    fs::path object_path = git_path / "objects" / hash.substr(0, 2) / hash.substr(2);
    /* cout << "object_path: " << object_path << endl; */
    string content = read_file(object_path);
    // remove EOF
    /* content.pop_back(); */
    /* cout << "Read(File After EOF Remove): " << content.length() << endl; */
    auto split_index = content.find('\0');
    /* cout << "Split index: " << split_index << endl; */
    // if (split_index == content.length() - 1) {
    //     cout<<"FFF"<<endl;
    //     //throw std::runtime_error("not a valid file");
    // }
    // it's too bad string_view cannot be used in STL algorithms
    /* string_view type(content.begin(),split_point); */
    string type = content.substr(0, split_index);
    string data = content.substr(split_index + 1);
    /* cout << "Read in Data Length: " << data.length() << endl; */
    /* cout << "Data: " << data << endl; */
    return create_object(type, data, git_path);
}

string write_object(GitObject* obj, bool write) {
    string total = obj->get_fmt() + '\0' + obj->to_filesystem();
    /* cout << "Write(String): " << total.length() << endl; */
    /* cout << total << endl; */

    /* string hash = SHA1::from_file("test"); */
    SHA1 checksum;
    checksum.update(total);
    string hash = checksum.final();
    /* cout << "Hash: " << hash << endl; */

    if (write) {
        fs::path object_path = obj->git_path / "objects" / hash.substr(0, 2) / hash.substr(2);
        if (!fs::exists(object_path)) write_file(object_path, total);
    }
    return hash;
}

string object_find(fs::path repo, fs::path obj, const string& fmt) { return obj; }

/* ********* Moving between project and git folders	********* */
string read_project_file_and_write_object(const fs::path git_path, const fs::path& file_path) {
    // read in and make GitBlob
    string content = read_file(file_path);
    GitBlob blob_obj = GitBlob(git_path, content);
    // write to object folder
    return write_object(&blob_obj);
}

void write_object_to_project_file(fs::path project_blob_path, string blob_hash) {
    fs::path git_path = repo_find(project_blob_path) / ".cpp-git";
    GitBlob blob_obj;
    read_into_object(blob_obj, git_path, blob_hash);

    write_file(project_blob_path, blob_obj.data);
}

string read_project_folder_and_write_tree(const fs::path& adding_directory, bool index) {
    fs::path project_base_path = repo_find(adding_directory);
    fs::path git_path = project_base_path / ".cpp-git";

    GitTree tree_obj(git_path);
#if 0
    /* cout << "DEBUG: " << "reading " << adding_directory << endl; */
    for (auto entry: fs::directory_iterator(adding_directory)){
        cout << entry.path() << endl;
    }
#endif
    for (auto entry : fs::directory_iterator(adding_directory)) {
        fs::path path = entry.path();
        if (is_git_repo(path)) {
            continue;
        }
        /* cout << "Entry path: " << path << endl; */
        if (fs::is_regular_file(entry)) {
            string blob_hash = read_project_file_and_write_object(git_path, path);
            tree_obj.add_entry("blob", path.filename(), blob_hash);
        } else if (fs::is_directory(path)) {
            string tree_hash = read_project_folder_and_write_tree(path);
            tree_obj.add_entry("tree", path.filename(), tree_hash);
        } else {
            throw std::runtime_error("cpp-git cannot handle this file");
        }
    }
    string output = write_object(&tree_obj);
    if (index) {
        /* cout << "Should write to index now" << endl; */
        write_file(git_path / "index", output);
    }
    return output;
}

/* ********* Traversal Helpers	********* */

// get back to project root
fs::path repo_find(fs::path file_path) {
    if (file_path.parent_path() == file_path) {
        throw std::runtime_error("No git directory");
    }
    if (fs::exists(file_path / ".cpp-git")) {
        return file_path;
    } else {
        return repo_find(file_path.parent_path());
    }
}

bool is_git_repo(const fs::path& path) {
    string name = path.filename();
    if (name == ".cpp-git") {
        return true;
    } else {
        return false;
    }
}
bool check_node_name(GitTreeNode& node, string file_it_name) { return node.name == file_it_name; }
bool end_of_path(typename fs::path::iterator file_it, typename fs::path::iterator end_it) {
    auto check_it = file_it;
    return (++check_it) == end_it;
}

void check_if_tree(GitTreeNode& node) {
    if (node.type != "tree") {
        throw std::runtime_error("this isn't a tree");
    }
}

void check_if_tree_exists(Option<GitTree> option_tree) {
    if (!option_tree.exists) {
        throw std::runtime_error("tree doesn't exist");
    }
}

GitTree* get_tree_from_hash(string hash, fs::path git_path) {
    GitObject* obj = read_object(git_path, hash);
    return dynamic_cast<GitTree*>(obj);
}

string path_relative_to_project(const fs::path project_base_path, fs::path entry_path) {
    string base = project_base_path.string();
    string entry = entry_path.string();

    auto [rel, _] = mismatch(entry.begin(), entry.end(), base.begin());
    auto idx = rel - entry.begin();

    return entry.substr(idx + 1);
}

bool is_in_set(const set<string>& set, string val) {
    auto it = set.find(val);
    if (it != set.end()) {
        return true;
    } else {
        return false;
    }
}
bool is_in_set(const map<string, string>& map, string val) {
    auto it = map.find(val);
    if (it != map.end()) {
        return true;
    } else {
        return false;
    }
}

/* ********* Finding Project File in GitTree	********* */
string find_hash_in_tree(GitTree* tree_obj, typename fs::path::iterator file_it,
                         const typename fs::path::iterator end_it, const fs::path git_path) {
    for (auto node : tree_obj->directory) {
        if (check_node_name(node, *file_it)) {
            bool end = end_of_path(file_it, end_it);
            if (end) {
                return node.hash;
            }
            // continue down
            else {
                file_it++;
                check_if_tree(node);
                GitTree subtree;
                read_into_object(subtree, git_path, node.hash);
                return find_hash_in_tree(&subtree, file_it, end_it, git_path);
            }
        }
    }
    return "";
}

GitBlob find_project_file_from_tree_hash(string tree_hash, fs::path relative_path,
                                         const fs::path git_path) {
    auto path_it = relative_path.begin();
    auto end_it = relative_path.end();
    GitTree root_tree;
    read_into_object(root_tree, git_path, tree_hash);
    string file_hash = find_hash_in_tree(&root_tree, path_it, end_it, git_path);
    GitBlob file_obj;
    read_into_object(file_obj, git_path, file_hash);
    return file_obj;
}

GitTree find_project_folder_from_tree(string tree_hash, fs::path relative_path,
                                      const fs::path git_path) {
    auto path_it = relative_path.begin();
    auto end_it = relative_path.end();
    /* cout << "Starting Folder Find" << endl; */
    GitTree root_tree;
    read_into_object(root_tree, git_path, tree_hash);
    string folder_hash = find_hash_in_tree(&root_tree, path_it, end_it, git_path);
    GitTree tree_obj;
    read_into_object(tree_obj, git_path, tree_hash);
    return tree_obj;
}

/* ********* Commit Helpers	********* */
// will only be direct if chkout a commit without a ref
// NOTE: instead of referencing a reference,
// just have HEAD point to a commit object instead
string get_commit_hash_from_branch(string full_branch_name, fs::path git_path) {
    if (fs::exists(git_path / full_branch_name)) {
        return read_file(git_path / full_branch_name);
    } else {
        std::cout << "Full branch name:" << full_branch_name << std::endl;
        throw std::runtime_error("Error. Not a valid branch");
    }
}

// will return refs/heads/branch_name
// or commit hash
string get_current_branch_full(fs::path git_path) {
    string content = read_file(git_path / "HEAD");
    auto idx = content.find(' ');
    if (idx != string::npos) {
        content.erase(0, idx + 1);
    }
    return content;
}

string get_full_branch_name(string branch_name) { return "refs/heads/" + branch_name; }

Option<GitTree> get_index_tree(fs::path git_path) {
    string tree_hash = read_file(git_path / "index");
    GitTree tree;
    bool option;
    if (tree_hash == "") {
        option = false;
        //return Option<GitTree>{tree, false};
    } else {
        read_into_object(tree, git_path, tree_hash);
        option = true;
        //return Option<GitTree>{tree, true};
    }
    return Option<GitTree>{tree,option};
}

GitCommit get_commit_from_hash(string commit_hash, fs::path git_path) {
    GitCommit commit_obj;
    read_into_object(commit_obj, git_path, commit_hash);
    return commit_obj;
}

Option<GitTree> get_head_tree(fs::path git_path) {
    string full_branch_name = get_current_branch_full(git_path);
    string commit_hash = get_commit_hash_from_branch(full_branch_name, git_path);
    GitTree tree;
    bool option;
    if (commit_hash == "") {
        option = false;
        //return Option<GitTree>{tree, false};
    } else {
        /* cout << "Commit Hash: " << tree_hash << endl; */
        GitCommit commit_obj = get_commit_from_hash(commit_hash, git_path);
        /* cout << "Commit tree: " << (commit_obj->tree_hash) << endl; */
        read_into_object(tree, git_path, commit_obj.tree_hash);
        option = true;
        //return Option<GitTree>{tree, true};
    }
    return Option<GitTree>{tree,option};
}
string get_tree_hash_of_index(fs::path git_path) {
    string content = read_file(git_path / "index");
    /* dereference_if_indirect(content); */
    /* GitObject* obj = read_object(project_base_path / ".cpp-git", content); */
    /* return dynamic_cast<GitTree*>(obj); */
    return content;
}
