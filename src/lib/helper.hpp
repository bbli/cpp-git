#ifndef HELPER_HPP
#define HELPER_HPP
#include <string>
#include <filesystem>
namespace fs = std::filesystem;

int test_function(void);

class Repo{
    private:

    public:
        fs::path worktree;
        fs::path gitdir;
        Repo(std::string path, bool force);
};
#endif
