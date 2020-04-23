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

void cmd_print(const std::vector<std::string> &tokens)
{
    for_each(tokens.begin(), tokens.end(), [](const std::string &s) { std::cout << s << std::endl; });
}

class InputParser
{
public:
    InputParser(int argc, char **argv)
    {
        this->cmd = argc <= 1 ? "" : argv[1];
        for (int i = 2; i < argc; ++i)
            this->args.push_back(std::string(argv[i]));
    }

    void execute_cmd()
    {
        this->menu[this->cmd](this->args);
    }

private:
    std::string cmd;
    std::vector<std::string> args = {};
    std::unordered_map<std::string, std::function<void(const std::vector<std::string> &args)>> menu = {
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
        {"tag",         cmd_print},
    };
};

int main(int argc, char **argv)
{

    /* ********* Scratch	********* */
    // test_function();
    /* try { */
    /*     auto val = fs::file_size("test.file"); */
    /*     std::cout << val << std::endl; */
    /* } catch (fs::filesystem_error& ex) { */
    /*     std::cout << ex.what() << '\n'; */
    /* } */
    /* for(auto &p:fs::directory_iterator(".")) */
    /*     std::cout << p.path().extension() << std::endl; */

    // constructs a path object, and checks if it exists in filesystem
    /* fs::path Path("src"); */
    /* std::cout << "Does path exists?" << fs::exists(Path) << std::endl; */
    // gets parent path of the path object
    /* std::cout << Path.parent_path() << std::endl; */
    // adds two paths together
    /* std::cout << Path / fs::path("hello") << std::endl; */

    /* ********* Main	********* */
    try
    {
        auto parser = InputParser(argc, argv);
        parser.execute_cmd();
    }
    catch (char const *e)
    {
        std::cout << e << std::endl;
    }
}
