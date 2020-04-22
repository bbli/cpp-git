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

void display_help(const std::vector<std::string> &tokens)
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
        if (this->cmds_with_relative_paths.find(cmd) != this->cmds_with_relative_paths.end())
            this->convert_relative_path_to_absolute();
    }

    void execute_cmd()
    {
        this->menu[this->cmd](this->args);
    }

private:
    void convert_relative_path_to_absolute()
    {
        std::filesystem::path pwd = std::filesystem::canonical(".");
        for (std::string &file : this->args)
        {
            if (file[0] != '/')
                file = pwd / file;
        }
    }
    std::string cmd;
    std::vector<std::string> args = {};
    std::set<std::string> cmds_with_relative_paths = {"add", "cat-file", "hash-object", "rm"};
    std::unordered_map<std::string, std::function<void(const std::vector<std::string> &args)>> menu = {
        {"", display_help},
        {"add", display_help},
        {"cat-file", display_help},
        {"checkout", display_help},
        {"commit", display_help},
        {"hash-object", display_help},
        {"init", display_help},
        {"log", display_help},
        {"ls-tree", display_help},
        {"merge", display_help},
        {"rebase", display_help},
        {"rev-parse", display_help},
        {"rm", display_help},
        {"show-ref", display_help},
        {"tag", display_help},
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
