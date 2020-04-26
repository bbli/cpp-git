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

std::string read_file(fs::path path);
void write_file(fs::path file_path, std::string message);
fs::path repo_find(fs::path file_path);
GitObject* readObject(fs::path git_path, std::string hash);
std::string writeObject(GitObject* obj, bool write = true);
GitTree* getIndexTree(fs::path git_path);
GitTree* get_head_tree(fs::path git_path);
GitTree* get_tree(std::string hash, fs::path git_path);
void printTree(fs::path git_path, std::string tree_hash);

bool isGitRepo(const fs::path& path);
bool checkNodeName(GitTreeNode& node, std::string file_it_name);
bool endOfPath(typename fs::path::iterator file_it, typename fs::path::iterator end_it);
void checkIfTree(GitTreeNode& node);

std::string getSubTreeHashForNewFile(GitTree* tree_obj, typename fs::path::iterator file_it,
                                const typename fs::path::iterator end_it, const fs::path git_path,
                                const fs::path& file_path);
std::string getSubTreeHashForNewFolder(GitTree* tree_obj, typename fs::path::iterator file_it,
                                  typename fs::path::iterator end_it, const fs::path git_path,
                                  const fs::path folder_path);

std::string readProjectFileAndWriteObject(const fs::path git_path, const fs::path& file_path);
std::string readProjectFolderAndWriteTree(const fs::path& adding_directory, bool index=false);
std::string path_relative_to_project(const fs::path project_base_path,fs::path entry_path);
bool is_in_set(const std::set<std::string>& set,std::string val);

void object_find(void);
#endif
