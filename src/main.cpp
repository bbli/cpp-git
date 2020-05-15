#include <commands.hpp>
#include <git_objects.hpp>
#include <functional>
#include <filesystem>
#include <iostream>
#include <unordered_map>
#include <string>
#include <cstring>
#include <set>
#include <vector>

using namespace std;

void cmd_print(const vector<string> &tokens)
{
    for_each(tokens.begin(), tokens.end(), [](const string &s) { cout << s << endl; });
}

class InputParser
{
public:
    InputParser(int argc, char **argv)
    {
        this->cmd = argc <= 1 ? "" : argv[1];
        for (int i = 2; i < argc; ++i)
            this->args.push_back(string(argv[i]));
    }

    void execute_cmd()
    {
        this->menu[this->cmd](this->args);
    }

private:
    string cmd;
    vector<string> args = {};
    unordered_map<string, function<void(const vector<string> &args)>> menu = {
        {"",            cmd_print},
        {"add",         cmd_add},
        {"cat-file",    cmd_cat_file},
        {"checkout",    cmd_checkout},
        {"commit",      cmd_commit},
        {"branch", cmd_branch},
        {"reset", cmd_reset},
        {"hash-object", cmd_hash_object},
        {"init",        cmd_init},
        {"log",         cmd_log},
        {"status",         cmd_status},
        /* {"ls-tree",     cmd_print}, */
        /* {"merge",       cmd_print}, */
        /* {"rebase",      cmd_print}, */
        /* {"rev-parse",   cmd_print}, */
        /* {"rm",          cmd_print}, */
        {"show-ref",    cmd_show_ref},
        {"tag",         cmd_tag}
    };
};

int main(int argc, char **argv)
{
    try
    {
        auto parser = InputParser(argc, argv);
        parser.execute_cmd();
    }
    catch (std::string& e)
    {
        cout << e << endl;
    }
}
