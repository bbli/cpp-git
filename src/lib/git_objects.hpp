#ifndef GIT_OBJECT_HPP
#define GIT_OBJECT_HPP
#include <filesystem>
#include <iostream>
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
    void to_internal(
        const std::string& data);             // filesystem -> lift to internal representation
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
    GitCommit(){};
    GitCommit(fs::path git_path, const std::string& data);
    GitCommit(fs::path git_path, std::string tree_hash, std::string parent_hash,
              std::string commit_message);
    // create a root commit without parent and any file
    GitCommit(fs::path git_path);
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
    typename std::string::const_iterator extract_one_tree_node(
        typename std::string::const_iterator start_it, const std::string& data);

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
    std::string commit_hash;
    std::string tag_message;
    virtual std::string get_fmt(void);
    GitTag(){};
    GitTag(fs::path git_path, const std::string& data);
    GitTag(fs::path git_path, const std::string commit_hash, const std::string tag_message);
    void to_internal(const std::string& data);
    virtual std::string to_filesystem(void);
};

class GitBlob : public GitObject {
   private:
   public:
    std::string data;
    virtual std::string get_fmt(void);
    GitBlob(){};
    GitBlob(fs::path git_path, const std::string& data);
    /* std::string get_fmt(void); */
    void to_internal(const std::string& data);
    virtual std::string to_filesystem(void);
};

#endif
