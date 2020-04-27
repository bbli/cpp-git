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
        {"add",         cmd_print},
        {"cat-file",    cmd_cat_file},
        {"checkout",    cmd_print},
        {"commit",      cmd_print},
        {"hash-object", cmd_print},
        {"init",        cmd_print},
        {"log",         cmd_print},
        {"ls-tree",     cmd_print},
        {"merge",       cmd_print},
        {"rebase",      cmd_print},
        {"rev-parse",   cmd_print},
        {"rm",          cmd_print},
        {"show-ref",    cmd_print},
        {"tag",         cmd_print}
    };
};

int main(int argc, char **argv)
{

    /* ********* Scratch	********* */
    // test_function();
    /* try { */
    /*     auto val = fs::file_size("test.file"); */
    /*     cout << val << endl; */
    /* } catch (fs::filesystem_error& ex) { */
    /*     cout << ex.what() << '\n'; */
    /* } */
    /* for(auto &p:fs::directory_iterator(".")) */
    /*     cout << p.path().extension() << endl; */

    // constructs a path object, and checks if it exists in filesystem
    /* fs::path Path("src"); */
    /* cout << "Does path exists?" << fs::exists(Path) << endl; */
    // gets parent path of the path object
    /* cout << Path.parent_path() << endl; */
    // adds two paths together
    /* cout << Path / fs::path("hello") << endl; */

    /* ********* Main	********* */
    try
    {
        auto parser = InputParser(argc, argv);
        parser.execute_cmd();
    }
    catch (char const *e)
    {
        cout << e << endl;
    }
}
