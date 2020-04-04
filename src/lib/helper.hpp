#ifndef HELPER_HPP
#define HELPER_HPP
#endif
#include <string>

int test_function(void);

class Repo{
    private:
        std::string worktree;
        std::string gitdir;

    public:
        Repo(std::string path, bool force);
};
