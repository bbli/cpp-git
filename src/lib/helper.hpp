#ifndef HELPER_HPP
#define HELPER_HPP
#include <filesystem>
/* #include <range/v3/view.hpp> */
#include <git_objects.hpp>
#include <string>
#include <set>
namespace fs = std::filesystem;
#include <iostream>
/* ********* Data Structures	********* */
// if force=true -> empty git directory
/* class Repo{ */
/*     private: */

/*     public: */
/*         fs::path worktree; */
/*         fs::path gitdir; */
/*         Repo(fs::path path, bool force); */
/* }; */

/* ********* Helper Functions	********* */
/* // T should be a range */
/* template<typename T> */
/* GitTreeNode createNode(T entry_parts){ */
/*     using namespace ranges; */
/*     for (auto [idx,part]: views::enumerate(entry_parts)){ */
/*         break; */
/*         std::cout << "Idx: " << idx << "Part: " << part << std::endl; */
/*         /1* if (idx==2){ *1/ */
/*         /1*     std::cout << "Should break" << std::endl; *1/ */
/*         /1*     break; *1/ */
/*         /1* } *1/ */
/*     } */
/* } */
template <typename T>
void printer(T container) {
    for (auto x : container) {
        std::cout << x << " ";
    }
    std::cout << std::endl;
}

/* File Operation */
std::string read_file(fs::path path);
void write_file(fs::path file_path, std::string message);

fs::path repo_find(fs::path file_path);

/* Object Operations */
GitObject* create_object(std::string type, std::string& data, fs::path git_path);
GitObject* read_object(fs::path git_path, std::string hash);
std::string write_object(GitObject* obj, bool write = true);
std::string object_find(fs::path repo, fs::path obj, const std::string& fmt);

std::string get_tree_hash_of_index(fs::path git_path);
GitTree* get_index_tree(fs::path git_path);
GitTree* get_head_tree(fs::path git_path);
GitTree* get_tree_from_hash(std::string hash, fs::path git_path);
void print_tree(fs::path git_path, std::string tree_hash);

bool is_git_repo(const fs::path& path);
bool check_node_name(GitTreeNode& node, std::string file_it_name);
bool end_of_path(typename fs::path::iterator file_it, typename fs::path::iterator end_it);
void check_if_tree(GitTreeNode& node);



std::string read_project_file_and_write_object(const fs::path git_path, const fs::path& file_path);
std::string read_project_folder_and_write_tree(const fs::path& adding_directory, bool index=false);
void write_object_to_project_file(fs::path project_blob_path, std::string blob_hash);
std::string path_relative_to_project(const fs::path project_base_path,fs::path entry_path);
bool is_in_set(const std::set<std::string>& set,std::string val);


#endif
