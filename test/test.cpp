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
        void SetUp() override{
            // Create and then read in the file
            null_string =  "ab cd ef\ngh ij kl";
            create_file(fs::current_path() / "file.txt",null_string);
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
/* ********* Git Init	********* */
// Test throw if not empty path
