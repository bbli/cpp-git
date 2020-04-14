#ifndef HELPER_HPP
#define HELPER_HPP
#include <string>
#include <filesystem>
namespace fs = std::filesystem;
/* ********* Data Structures	********* */
//if force=true -> empty git directory
/* class Repo{ */
/*     private: */

/*     public: */
/*         fs::path worktree; */
/*         fs::path gitdir; */
/*         Repo(fs::path path, bool force); */
/* }; */

class GitObject{
    private:
    public:
        fs::path git_path;
        /* std::string data; */
        /* const static inline std::string fmt; */
        GitObject(fs::path git_path);
        virtual void to_internal(const std::string& data)=0; //change internal representation to filesystem
        virtual std::string to_filesystem(void)=0; //change filesystem to internal representation
        virtual std::string get_fmt(void)=0;
};

class GitCommit: public GitObject{
    private:
    public:
        virtual std::string get_fmt(void);
        GitCommit(fs::path git_path,const std::string& data);
        virtual void to_internal(const std::string& data);
        virtual std::string to_filesystem(void);
};

class GitTree: public GitObject{
    private:
    public:
        virtual std::string get_fmt(void);
        GitTree(fs::path git_path,const std::string& data);
        virtual void to_internal(const std::string& data);
        virtual std::string to_filesystem(void);
};

class GitTag: public GitObject{
    private:
    public:
        virtual std::string get_fmt(void);
        GitTag(fs::path git_path,const std::string& data);
        virtual void to_internal(const std::string& data);
        virtual std::string to_filesystem(void);
};

class GitBlob: public GitObject{
    private:
    public:
        std::string data;
        virtual std::string get_fmt(void);
        GitBlob(fs::path git_path,const std::string& data);
        /* std::string get_fmt(void); */
        virtual void to_internal(const std::string& data);
        virtual std::string to_filesystem(void);
};
/* ********* Helper Functions	********* */


void create_file(fs::path file_path,std::string message);
fs::path repo_find(fs::path file_path);
GitObject* readObject(fs::path git_path, std::string hash);
std::string writeObject(GitObject& obj,bool write=true);

void object_find(void);
#endif
