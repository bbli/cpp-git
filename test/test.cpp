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
    /* read_result.pop_back(); */
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

class TreeTraversal: public ::testing::Test{
    protected:
        std::string message_1;
        std::string message_2;
        std::string tree_hash;
        fs::path current_path;
        GitTree tree_obj;
        void SetUp() override{
            //Set up the repo
            current_path = fs::current_path();
            if (!fs::exists(current_path)){
                git_init();
            }
            fs::path git_path = current_path / ".cpp-git";

            // Create a tree
            message_1 = "file1";
            message_2 = "file2";
            GitBlob file_1(git_path,message_1);
            GitBlob file_2(git_path, message_2);
            /* std::cout << "File 1:" << std::endl; */
            std::string hash_1 = writeObject(&file_1);
            /* std::cout << "File 2:" << std::endl; */
            std::string hash_2 = writeObject(&file_2);
            tree_obj = GitTree(git_path);
            tree_obj.add_entry("blob","file1.txt",hash_1);
            tree_obj.add_entry("blob","file2.txt",hash_2);
            /* std::cout << "Tree: " << std::endl; */
            tree_hash = writeObject(&tree_obj);
        }
};

TEST_F(TreeTraversal, one_level_tree){
    /* // Now walk */
    fs::path git_path = repo_find(fs::current_path()) / ".cpp-git";
    chkout_obj(git_path,tree_hash);

    // Check that we have written to project's path and that content is the same
    std::string final_message_1 = read_file(current_path / "file1.txt");
    /* final_message_1.pop_back(); */
    std::string final_message_2 = read_file(current_path / "file2.txt");
    /* final_message_2.pop_back(); */
    ASSERT_EQ(message_1,final_message_1);
    ASSERT_EQ(message_2,final_message_2);
}

TEST_F(TreeTraversal, two_level_tree){
    // Create root tree
    fs::path git_path = repo_find(fs::current_path()) / ".cpp-git";
    std::string message_3 = "file3";
    GitBlob file_3(git_path,message_3);
    std::string hash_3 = writeObject(&file_3);
    GitTree root_tree_obj(git_path);
    // Take tree from above and append to this tree    
    root_tree_obj.add_entry("blob","file3.txt",hash_3);
    root_tree_obj.add_entry("tree","folder",tree_hash);
    std::string root_hash = writeObject(&root_tree_obj);

    // Now walk
    chkout_obj(git_path,root_hash);

    // Check that everything is the same
    ASSERT_EQ(message_3,read_file(current_path / "file3.txt"));
    std::string final_message_1 = read_file(current_path / "folder" / "file1.txt");
    std::string final_message_2 = read_file(current_path / "folder" / "file2.txt");
    ASSERT_EQ(message_1,final_message_1);
    ASSERT_EQ(message_2,final_message_2);
}


#if 1
// Turned off for now b/c git_add always adds
// from project base right now
TEST(Staging, basePath_git_add){
    // NOTE: this will not be a faitful representation,as we are
    // mapping from within test_stage folder
    // This is done since current_directory is a build folder for extra things
    fs::path project_base_path = repo_find(fs::current_path());

    fs::path test_stage_path  = project_base_path / "test_stage";
    if(!fs::exists(test_stage_path)){
        fs::create_directory(test_stage_path);
    }
    write_file(test_stage_path / "stage1.txt","stage1");
    write_file(test_stage_path / "stage2.txt","stage2");
    write_file(test_stage_path / "fold" / "stage3.txt","stage3");
    std::string stage_tree_hash = createIndexTreeFromFolder(test_stage_path,true);
    std::string index_hash = read_file(project_base_path / ".cpp-git" / "index");
    ASSERT_EQ(index_hash,stage_tree_hash);
}
#endif

TEST(Staging, dereference_if_indirect){
    std::string test_string = "ref: refs/heads/master";
    dereference_if_indirect(test_string);
    std::cout << "Test string: " << test_string << std::endl;
    ASSERT_EQ(test_string,std::string("refs/heads/master"));
}

TEST(Staging, git_add_file_oneLevelOneModifiedFile){
    // Create a folder + files -> write to tree
    fs::path project_base_path = repo_find(fs::current_path());
    fs::path git_path = project_base_path / ".cpp-git";

    fs::path test_stage_path  = project_base_path / "oneLevelOneModifiedFile";
    if(!fs::exists(test_stage_path)){
        fs::create_directory(test_stage_path);
    }
    write_file(test_stage_path / "stage1.txt","stage1");
    write_file(test_stage_path / "stage2.txt","stage2");

    std::string tree_hash = createIndexTreeFromFolder(test_stage_path,true);
    std::cout << "Old Tree Hash" << std::endl;
    printTree(tree_hash);
    write_file(project_base_path / ".cpp-git" / "HEAD",tree_hash);
    // Modify stage2.txt and then add to index
    write_file(test_stage_path / "stage2.txt", "changed stage2");
    std::string new_tree_hash = git_add_file(test_stage_path / "stage2.txt",true);
    // compare the two tree objects
    std::cout << "New Tree Hash" << std::endl;
    printTree(new_tree_hash);

}
/* ********* Git Init	********* */
// Test throw if not empty path
