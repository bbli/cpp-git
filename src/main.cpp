#include <iostream>
#include <commands.hpp>

#include <helper.hpp>


int main(void) {
    /* ********* Scratch	********* */
    test_function();
    /* try { */
    /*     auto val = fs::file_size("test.file"); */
    /*     std::cout << val << std::endl; */
    /* } catch (fs::filesystem_error& ex) { */
    /*     std::cout << ex.what() << '\n'; */
    /* } */
    /* for(auto &p:fs::directory_iterator(".")) */
    /*     std::cout << p.path().extension() << std::endl; */

    //constructs a path object, and checks if it exists in filesystem
    /* fs::path Path("src"); */
    /* std::cout << "Does path exists?" << fs::exists(Path) << std::endl; */
    //gets parent path of the path object
    /* std::cout << Path.parent_path() << std::endl; */
    //adds two paths together
    /* std::cout << Path / fs::path("hello") << std::endl; */


    /* ********* Main	********* */
    try{
        fs::path project_path = repo_find(fs::current_path());
        fs::path git_path = project_path / ".cpp-git";
        std::cout << "Project path " << project_path << std::endl;

        GitBlob blob(git_path," version");
        std::string return_hash = writeObject(&blob);


        std::cout << "Return hash: " << return_hash << std::endl;
        GitObject* gitobject = readObject(git_path,return_hash);
        std::cout << "Data: " << gitobject->to_filesystem() << std::endl;
    } catch (char const*e){
        std::cout << e << std::endl;
    }

}
