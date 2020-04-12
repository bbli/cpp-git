#ifndef HELPER_HPP
#define HELPER_HPP
#include <string>
#include <filesystem>
namespace fs = std::filesystem;
/* ********* Data Structures	********* */
//if force=true -> empty git directory
class Repo{
    private:

    public:
        fs::path worktree;
        fs::path gitdir;
        Repo(fs::path path, bool force);
};

/* ********* Helper Functions	********* */


void create_file(fs::path file_path,std::string message);
fs::path repo_find(fs::path file_path,bool required);

#endif
