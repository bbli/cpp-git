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
        std::string data;
        const static inline std::string fmt;
        GitObject(fs::path git_path,std::string& data);
        void serialize(void);
        void deserialize(void);
};

class GitCommit: public GitObject{
    private:
    public:
        const static inline std::string fmt = "commit";
        GitCommit(fs::path git_path,std::string data):GitObject( git_path ,data){};
};

class GitTree: public GitObject{
    private:
    public:
        const static inline std::string fmt = "tree";
        GitTree(fs::path git_path,std::string data):GitObject( git_path ,data){};
};

class GitTag: public GitObject{
    private:
    public:
        const static inline std::string fmt = "tag";
        GitTag(fs::path git_path,std::string data):GitObject( git_path ,data){};
};

class GitBlob: public GitObject{
    private:
    public:
        const static inline std::string fmt = "blob";
        GitBlob(fs::path git_path,std::string data):GitObject( git_path ,data){};
};
/* ********* Helper Functions	********* */


void create_file(fs::path file_path,std::string message);
GitObject object_read(fs::path git_path, std::string hash);

fs::path repo_find(fs::path file_path,bool required);
void object_find(void);
#endif
