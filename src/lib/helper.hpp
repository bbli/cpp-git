#ifndef HELPER_HPP
#define HELPER_HPP
#include <filesystem>
#include <iostream>
#include <range/v3/view.hpp>
#include <string>
#include <vector>
namespace fs = std::filesystem;
/* ********* Data Structures	********* */
// if force=true -> empty git directory
/* class Repo{ */
/*     private: */

/*     public: */
/*         fs::path worktree; */
/*         fs::path gitdir; */
/*         Repo(fs::path path, bool force); */
/* }; */

class GitObject {
   private:
   public:
    fs::path git_path;
    /* std::string data; */
    /* const static inline std::string fmt; */
    GitObject(){};
    GitObject(fs::path git_path);
    virtual void to_internal(
        const std::string& data) = 0;             // filesystem -> lift to internal representation
    virtual std::string to_filesystem(void) = 0;  // internal representation -> write to filesystem
    virtual std::string get_fmt(void) = 0;
};

class GitCommit : public GitObject {
   private:
   public:
    std::string tree_hash;
    std::string parent_hash;
    std::string commit_message;
    virtual std::string get_fmt(void);
    GitCommit(fs::path git_path, const std::string& data);
    void to_internal(const std::string& data);
    virtual std::string to_filesystem(void);
};

struct GitTreeNode {
    std::string type;
    std::string name;
    std::string hash;
    friend std::ostream& operator<<(std::ostream& os, GitTreeNode& t);
};

class GitTree : public GitObject {
   private:
   public:
    std::vector<GitTreeNode> directory;
    virtual std::string get_fmt(void);
    GitTree(){};
    GitTree(fs::path git_path, const std::string& data);
    GitTree(fs::path git_path);
    void to_internal(const std::string& data);
    virtual std::string to_filesystem(void);
    void add_entry(std::string type, std::string file_name, std::string hash);
};

class GitTag : public GitObject {
   private:
   public:
    virtual std::string get_fmt(void);
    GitTag(fs::path git_path, const std::string& data);
    void to_internal(const std::string& data);
    virtual std::string to_filesystem(void);
};

class GitBlob : public GitObject {
   private:
   public:
    std::string data;
    virtual std::string get_fmt(void);
    GitBlob(fs::path git_path, const std::string& data);
    /* std::string get_fmt(void); */
    void to_internal(const std::string& data);
    virtual std::string to_filesystem(void);
};
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

std::string read_file(fs::path path);
void write_file(fs::path file_path, std::string message);
fs::path repo_find(fs::path file_path);
GitObject* readObject(fs::path git_path, std::string hash);
std::string writeObject(GitObject* obj, bool write = true);
std::string getTreeHashOfIndex(fs::path git_path);
void printTree(fs::path git_path, std::string tree_hash);
bool isGitRepo(const fs::path& path);
std::string readFileAndWriteObject(const fs::path git_path, const fs::path& file_path);

void object_find(void);
#endif
