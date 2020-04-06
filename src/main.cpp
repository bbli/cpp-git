#include <iostream>
#include <filesystem>

#include <helper.hpp>
#include "main.hpp"
namespace fs = std::filesystem;

int main(void) {
    test_function();
    try {
        auto val = fs::file_size("test.file");
        std::cout << val << std::endl;
    } catch (fs::filesystem_error& ex) {
        std::cout << ex.what() << '\n';
    }
    for(auto &p:fs::directory_iterator("."))
        std::cout << p.path().extension() << std::endl;

    //constructs a path object
    fs::path Path("src/EXEC");
    //checks that the path is valid
    std::cout << fs::exists(Path) << std::endl;
    //gets parent path of the path object
    std::cout << Path.parent_path() << std::endl;
    //adds two paths together
    std::cout << Path / fs::path("hello") << std::endl;

}
