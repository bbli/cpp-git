#include <helper.hpp>
#include <helper.cpp>
#include <commands.hpp>
#include <gtest/gtest.h>
#include <iostream>
#include <range/v3/algorithm.hpp>
#include <vector>

using namespace ranges;
/* TEST(Suite,test1){ */
/*     EXPECT_EQ(test_function(),0); */
/* } */
// sd

TEST(Helper, does_write_file_overwrite){
    fs::path file_path = fs::current_path() / "overwrite.txt";
    std::string message = "one";
    write_file(file_path,message);
    /* std::cout << "Before overwrite: " << read_file(file_path) << std::endl; */
    std::string message2 = "two";
    write_file(file_path,message2);
    std::string read_result = read_file(file_path);
    // get rid of EOF
    read_result.pop_back();
    /* std::cout << "After overwrite: " << read_result << std::endl; */
    ASSERT_EQ(message2, read_result);
}

TEST(Scratch, basic_range){
    auto new_range = "ab\ncd" | views::split('\n');
    std::cout << new_range << std::endl;
}
TEST(Scratch, nested_range){
    auto test_range = "ab cd ef\ngh ij kl" | views::split('\n');
    std::cout << test_range << std::endl;
    for (auto x: test_range){
        auto x_parts =  x | views::split(' ');
        for (auto s:x_parts){
            /* auto cnt = ranges::count_if(s, [](const auto& s) {return true;} ); */
            /* std::cout << cnt << std::endl; */
        }
        std::cout << "New range: " << x_parts << std::endl;
    }
}

TEST(Scratch, nest_vector_range){
    std::string null_string =  "ab cd ef\ngh ij kl";
    std::cout << "Length of string: " << null_string.length() << std::endl;
    std::vector<char> no_null(null_string.begin(),null_string.end());
    std::cout << "Length of vector:" << no_null.size() << std::endl;

    auto test_range = no_null | views::split('\n');
    std::cout << test_range << std::endl;
    for (auto x: test_range){
        auto x_parts =  x | views::split(' ');
        std::cout << "New range: " << x_parts << std::endl;
    }
}

class GitTreeTest: public ::testing::Test{
    protected:
        std::string null_string;
        std::string content;
        std::vector<char> no_null;
        fs::path git_path;
        void SetUp() override{
            git_path = fs::current_path() / ".cpp-git";
            // Create and then read in the file
            null_string =  "ab cd ef\ngh ij kl";
            write_file(fs::current_path() / "file.txt",null_string);
            content = read_file("file.txt");
            no_null = std::vector(content.begin(),content.end());
            no_null.pop_back(); 
        }
};

TEST_F(GitTreeTest, reading_file_EOF_check){
    /* std::cout << (std::vector(content.begin(),content.end())[5] == EOF)<< std::endl; */
    std::cout << "Length of original string: " << 17  << std::endl;
    std::cout << "Length of string: " << content.length() << std::endl;
    std::cout << " Length of vector: " << no_null.size() << std::endl;
    std::cout << "String: ";
    printer(content);
    std::cout << "Vector: ";
    printer(no_null);
}

TEST_F(GitTreeTest, enumerate_range_check){
    auto entry_list = no_null | views::split('\n');
    std::cout << entry_list << std::endl;
    for (auto entry: entry_list){
        auto entry_parts =  entry | views::split(' ');
        /* createNode(entry_parts); */
        for (auto [idx,part]: views::enumerate(entry_parts)){
            std::cout << "Idx: " << idx << "Part: " << part << std::endl;
        }
    }
}

TEST_F(GitTreeTest, to_internal){
    GitTree obj(fs::current_path(),content);
}

TEST_F(GitTreeTest, to_internal_and_to_filesystem){
    // check that reading from 
    std::string data = "blob apple.txt dfa11";
    GitTree tree_obj(git_path,data);
    ASSERT_EQ(data,(&tree_obj)->to_filesystem());

}

TEST_F(GitTreeTest, writeObject_and_readObject){
    // check that writing and then reading an object 
    // is still the same
    std::string message = "blob apple.txt dfa11\nblob orange.txt k12aq";
    std::cout << "Original Data Length: " << message.length() << std::endl;
    std::cout << message << std::endl;

    GitTree tree_obj(git_path,message);
    std::string hash = writeObject(&tree_obj);
    GitObject* read_result = readObject(git_path,hash);
    ASSERT_EQ(hash,writeObject(read_result,false));
}

TEST(GitBlob, to_internal_and_to_filesystem){
    std::string data = "#include <iostream>";
    GitBlob blob_obj(fs::current_path() / ".cpp-git", data);
    ASSERT_EQ(data,(&blob_obj)->to_filesystem());
}

TEST(GitCommit, to_internal_and_to_filesystem){
    std::string data = "jk18da\nua914q\nFirst commit";
    GitCommit commit_obj(fs::current_path() / ".cpp-git", data);
    ASSERT_EQ(data,(&commit_obj)->to_filesystem());
}
/* ********* Git Init	********* */
// Test throw if not empty path
