#include <gtest/gtest.h>

#include <commands.hpp>
#include <helper.cpp>
#include <helper.hpp>
#include <iostream>
#include <range/v3/algorithm.hpp>
#include <range/v3/view.hpp>
#include <vector>

using namespace ranges;
/* TEST(Suite,test1){ */
/*     EXPECT_EQ(test_function(),0); */
/* } */
// sd

void git_folder_setup(const std::string& folder_name) {
    // make folder
    fs::path folder_path = fs::current_path() / folder_name;
    /* std::cout << "Folder path: " << folder_path << std::endl; */
    if (fs::exists(folder_path)) {
        fs::remove_all(folder_path);
    }
    // initialize within that folder
    git_init(folder_path);
}

TEST(Helper, does_write_file_overwrite) {
    fs::path file_path = fs::current_path() / "overwrite.txt";
    std::string message = "one";
    write_file(file_path, message);
    /* std::cout << "Before overwrite: " << read_file(file_path) << std::endl; */
    std::string message2 = "two";
    write_file(file_path, message2);
    std::string read_result = read_file(file_path);
    // get rid of EOF
    /* read_result.pop_back(); */
    /* std::cout << "After overwrite: " << read_result << std::endl; */
    ASSERT_EQ(message2, read_result);
}

TEST(Scratch, system_unix_diff){
    fs::path current_path = fs::current_path();
    write_file(current_path / "diff1.txt", "abc");
    write_file(current_path / "diff2.txt", "def");
    system("diff --color diff1.txt diff2.txt");

    fs::remove(current_path / "diff1.txt");
    fs::remove(current_path / "diff2.txt");
}

// Answer: Does not skip hidden files
TEST(Scratch, directory_iterator_hidden_files) {
    fs::path folder_path = fs::current_path() / "test_hiddenFiles";
    fs::create_directory(folder_path);
    write_file(folder_path / "test.txt", "test");
    write_file(folder_path / ".a.txt", "a");
    fs::create_directory(folder_path / ".hidden");

    for (auto entry : fs::directory_iterator(folder_path)) {
        std::cout << entry << std::endl;
    }

    fs::remove_all(folder_path);
}

TEST(Scratch, basic_range) {
    auto new_range = "ab\ncd" | views::split('\n');
    std::cout << new_range << std::endl;
}
TEST(Scratch, nested_range) {
    auto test_range = "ab cd ef\ngh ij kl" | views::split('\n');
    std::cout << test_range << std::endl;
    for (auto x : test_range) {
        auto x_parts = x | views::split(' ');
        /* for (auto s : x_parts) { */
            /* auto cnt = ranges::count_if(s, [](const auto& s) {return true;} ); */
            /* std::cout << cnt << std::endl; */
        /* } */
        std::cout << "New range: " << x_parts << std::endl;
    }
}
TEST(Scratch, enumerate_range_seeWhichComesFirst) {
    auto test_range = "ab cd ef\ngh ij kl" | views::split('\n');
    /* std::cout << test_range << std::endl; */
    for (auto [idx, part] : views::enumerate(test_range)) {
        std::cout << "Idx: " << idx << "Part: " << part << std::endl;
    }
}

TEST(Scratch, nest_vector_range) {
    std::string null_string = "ab cd ef\ngh ij kl";
    std::cout << "Length of string: " << null_string.length() << std::endl;
    std::vector<char> no_null(null_string.begin(), null_string.end());
    std::cout << "Length of vector:" << no_null.size() << std::endl;

    auto test_range = no_null | views::split('\n');
    std::cout << test_range << std::endl;
    for (auto x : test_range) {
        auto x_parts = x | views::split(' ');
        std::cout << "New range: " << x_parts << std::endl;
    }
}

class GitTreeTest : public ::testing::Test {
   protected:
    std::string null_string;
    std::string content;
    std::vector<char> no_null;
    fs::path git_path;
    void SetUp() override {
        git_path = fs::current_path() / ".cpp-git";
        // Create and then read in the file
        null_string = "ab cd ef\ngh ij kl";
        write_file(fs::current_path() / "file.txt", null_string);
        content = read_file("file.txt");
        no_null = std::vector(content.begin(), content.end());
    }
};

TEST_F(GitTreeTest, read_file_eofCheck) {
    /* std::cout << (std::vector(content.begin(),content.end())[5] == EOF)<< std::endl; */
    ASSERT_EQ(17, content.length());
    /* std::cout << "Length of original string: " << 17  << std::endl; */
    /* std::cout << "Length of string: " << content.length() << std::endl; */
    /* printer(content); */
}

TEST_F(GitTreeTest, to_internal) { GitTree obj(fs::current_path(), content); }

TEST_F(GitTreeTest, to_internal_and_to_filesystem) {
    // check that reading from
    std::string data = "blob apple.txt dfa11";
    GitTree tree_obj(git_path, data);
    ASSERT_EQ(data, (&tree_obj)->to_filesystem());
}

TEST_F(GitTreeTest, writeObject_and_readObject) {
    // check that writing and then reading an object
    // is still the same
    std::string message = "blob apple.txt dfa11\nblob orange.txt k12aq";
    std::cout << "Original Data Length: " << message.length() << std::endl;
    std::cout << message << std::endl;

    GitTree tree_obj(git_path, message);
    std::string hash = writeObject(&tree_obj);
    GitObject* read_result = readObject(git_path, hash);
    ASSERT_EQ(hash, writeObject(read_result, false));
}

TEST(GitBlob, to_internal_and_to_filesystem) {
    std::string data = "#include <iostream>";
    GitBlob blob_obj(fs::current_path() / ".cpp-git", data);
    ASSERT_EQ(data, (&blob_obj)->to_filesystem());
}

TEST(GitCommit, to_internal_and_to_filesystem) {
    std::string data = "jk18da\nua914q\nFirst commit";
    GitCommit commit_obj(fs::current_path() / ".cpp-git", data);
    ASSERT_EQ(data, (&commit_obj)->to_filesystem());
}

class TreeTraversal : public ::testing::Test {
   protected:
    std::string message_1;
    std::string message_2;
    std::string tree_hash;
    fs::path current_path;
    GitTree tree_obj;
    void SetUp() override {
        // Set up the repo
        current_path = fs::current_path();
        if (!fs::exists(current_path)) {
            git_init(current_path);
        }
        fs::path git_path = current_path / ".cpp-git";

        // Create a tree
        message_1 = "file1";
        message_2 = "file2";
        GitBlob file_1(git_path, message_1);
        GitBlob file_2(git_path, message_2);
        /* std::cout << "File 1:" << std::endl; */
        std::string hash_1 = writeObject(&file_1);
        /* std::cout << "File 2:" << std::endl; */
        std::string hash_2 = writeObject(&file_2);
        tree_obj = GitTree(git_path);
        tree_obj.add_entry("blob", "file1.txt", hash_1);
        tree_obj.add_entry("blob", "file2.txt", hash_2);
        /* std::cout << "Tree: " << std::endl; */
        tree_hash = writeObject(&tree_obj);
    }
};

TEST_F(TreeTraversal, one_level_tree) {
    /* // Now walk */
    fs::path git_path = repo_find(fs::current_path()) / ".cpp-git";
    chkout_obj(git_path, tree_hash);

    // Check that we have written to project's path and that content is the same
    std::string final_message_1 = read_file(current_path / "file1.txt");
    /* final_message_1.pop_back(); */
    std::string final_message_2 = read_file(current_path / "file2.txt");
    /* final_message_2.pop_back(); */
    ASSERT_EQ(message_1, final_message_1);
    ASSERT_EQ(message_2, final_message_2);
}

TEST_F(TreeTraversal, two_level_tree) {
    // Create root tree
    fs::path git_path = repo_find(fs::current_path()) / ".cpp-git";
    std::string message_3 = "file3";
    GitBlob file_3(git_path, message_3);
    std::string hash_3 = writeObject(&file_3);
    GitTree root_tree_obj(git_path);
    // Take tree from above and append to this tree
    root_tree_obj.add_entry("blob", "file3.txt", hash_3);
    root_tree_obj.add_entry("tree", "folder", tree_hash);
    std::string root_hash = writeObject(&root_tree_obj);

    // Now walk
    chkout_obj(git_path, root_hash);

    // Check that everything is the same
    ASSERT_EQ(message_3, read_file(current_path / "file3.txt"));
    std::string final_message_1 = read_file(current_path / "folder" / "file1.txt");
    std::string final_message_2 = read_file(current_path / "folder" / "file2.txt");
    ASSERT_EQ(message_1, final_message_1);
    ASSERT_EQ(message_2, final_message_2);
}

#if 1
TEST(Staging, git_add_entireFolder) {
    git_folder_setup("entireFolder");
    fs::path project_base_path = repo_find(fs::current_path() / "entireFolder");
    std::cout << "Project base path: " << project_base_path << std::endl;
    fs::path git_path = project_base_path / ".cpp-git";

    write_file(project_base_path / "stage1.txt", "stage1");
    write_file(project_base_path / "stage2.txt", "stage2");
    write_file(project_base_path / "fold" / "stage3.txt", "stage3");
    std::string stage_tree_hash = readProjectFolderAndWriteTree(project_base_path, true);
    std::string index_hash = read_file(project_base_path / ".cpp-git" / "index");
    ASSERT_EQ(index_hash, stage_tree_hash);
}
#endif

TEST(Staging, dereference_if_indirect) {
    std::string folder_name = "dereference_if_indirect";
    git_folder_setup(folder_name);
    fs::path project_base_path = repo_find(fs::current_path() / folder_name);
    std::cout << "Project base path: " << project_base_path << std::endl;
    fs::path git_path = project_base_path / ".cpp-git";
    
    write_file(git_path / "refs/heads/master","abc");

    std::string test_string = "ref: refs/heads/master";
    std::string commit_hash = dereference_if_indirect(test_string,git_path);
    std::cout << "Test string: " << commit_hash << std::endl;
    ASSERT_EQ(commit_hash,"abc");
}

TEST(Staging, git_add_file_oneLevelOneModifiedFile) {
    git_folder_setup("oneLevelOneModifiedFile");
    fs::path project_base_path = repo_find(fs::current_path() / "oneLevelOneModifiedFile");
    std::cout << "Project base path: " << project_base_path << std::endl;
    fs::path git_path = project_base_path / ".cpp-git";

    // Create a folder + files -> write to tree
    write_file(project_base_path / "stage1.txt", "stage1");
    write_file(project_base_path / "stage2.txt", "stage2");

    std::string tree_hash = readProjectFolderAndWriteTree(project_base_path, true);
    std::cout << "Old Tree Hash" << std::endl;
    printTree(git_path, tree_hash);
    /* write_file(project_base_path / ".cpp-git" / "HEAD", tree_hash); */
    // Modify stage2.txt and then add to index
    write_file(project_base_path / "stage2.txt", "changed stage2");
    std::string new_tree_hash = git_add_file(project_base_path / "stage2.txt");
    // compare the two tree objects
    std::cout << "New Tree Hash" << std::endl;
    printTree(git_path, new_tree_hash);

    fs::remove_all(project_base_path);
}

TEST(Staging, git_add_file_oneLevelNewFile) {
    git_folder_setup("oneLevelNewFile");
    fs::path project_base_path = repo_find(fs::current_path() / "oneLevelNewFile");
    std::cout << "Project base path: " << project_base_path << std::endl;
    fs::path git_path = project_base_path / ".cpp-git";

    // Create new files -> write to tree
    write_file(project_base_path / "stage1.txt", "stage1");
    write_file(project_base_path / "stage2.txt", "stage2");

    std::string tree_hash = readProjectFolderAndWriteTree(project_base_path, true);
    std::cout << "Old Tree Listing" << std::endl;
    printTree(git_path, tree_hash);
    /* write_file(project_base_path / ".cpp-git" / "HEAD", tree_hash); */
    // Modify stage2.txt and then add to index
    write_file(project_base_path / "stage3.txt", "stage3");
    std::string new_tree_hash = git_add_file(project_base_path / "stage3.txt");
    // compare the two tree objects
    std::cout << "New Tree Listing" << std::endl;
    printTree(git_path, new_tree_hash);
    fs::remove_all(project_base_path);
}

TEST(Scratch, print_relative_path) {
    fs::path relative_path = "folder/a.txt";
    std::cout << relative_path << std::endl;
}

#if 1
TEST(Staging, git_add_file_twoLevelsNewFile) {
    git_folder_setup("twoLevelsNewFile");
    fs::path project_base_path = repo_find(fs::current_path() / "twoLevelsNewFile");
    std::cout << "Project base path: " << project_base_path << std::endl;
    fs::path git_path = project_base_path / ".cpp-git";

    // Create new files -> write to tree
    write_file(project_base_path / "stage1.txt", "stage1");
    write_file(project_base_path / "stage2.txt", "stage2");
    fs::create_directory(project_base_path / "folder");
    write_file(project_base_path / "folder" / "stage3.txt", "stage3");

    std::string tree_hash = readProjectFolderAndWriteTree(project_base_path, true);
    /* std::cout << "Index hash: " << tree_hash << std::endl; */
    /* std::cout << "Old Tree Listing" << std::endl; */
    /* printTree(git_path,tree_hash); */
    /* write_file(project_base_path / ".cpp-git" / "HEAD", tree_hash); */

    // Add new file under folder
    write_file(project_base_path / "folder" / "stage4.txt", "stage4");
    std::string new_tree_hash = git_add_file(project_base_path / "folder" / "stage4.txt");
    std::cout << "add_file new_tree_hash: " << new_tree_hash << std::endl;
    // compare the two tree objects
    /* std::cout << "New Tree Listing" << std::endl; */
    /* printTree(git_path,new_tree_hash); */
    GitBlob* stage4_blob = findProjectFileFromTree(new_tree_hash, "folder/stage4.txt", git_path);
    ASSERT_EQ(stage4_blob->data, "stage4");

    fs::remove_all(project_base_path);
}

TEST(Staging, git_add_folder_twoLevelsNewFile) {
    git_folder_setup("add_folder_twoLevelsNewFile");
    fs::path project_base_path = repo_find(fs::current_path() / "add_folder_twoLevelsNewFile");
    std::cout << "Project base path: " << project_base_path << std::endl;
    fs::path git_path = project_base_path / ".cpp-git";

    // Create new files -> write to tree
    write_file(project_base_path / "stage1.txt", "stage1");
    write_file(project_base_path / "stage2.txt", "stage2");
    fs::create_directory(project_base_path / "folder");
    write_file(project_base_path / "folder" / "stage3.txt", "stage3");

    std::string tree_hash = readProjectFolderAndWriteTree(project_base_path, true);
    /* std::cout << "Index hash: " << tree_hash << std::endl; */
    /* std::cout << "Old Tree Listing" << std::endl; */
    /* printTree(git_path,tree_hash); */
    /* write_file(project_base_path / ".cpp-git" / "HEAD", tree_hash); */

    // Add new file under folder
    write_file(project_base_path / "folder" / "stage4.txt", "stage4");
    std::string new_tree_hash = git_add_folder(project_base_path / "folder");

    GitBlob* stage4_blob = findProjectFileFromTree(new_tree_hash, "folder/stage4.txt", git_path);
    ASSERT_EQ(stage4_blob->data, "stage4");
    GitTree* folder = findProjectFolderFromTree(new_tree_hash, "folder", git_path);
    std::cout << "folder listing: " << std::endl;
    printer(folder->directory);

    fs::remove_all(project_base_path);
}

TEST(Staging, git_add_folder_twoLevelsNewFolder) {
    git_folder_setup("add_folder_twoLevelsNewFile");
    fs::path project_base_path = repo_find(fs::current_path() / "add_folder_twoLevelsNewFile");
    std::cout << "Project base path: " << project_base_path << std::endl;
    fs::path git_path = project_base_path / ".cpp-git";

    // Create new files -> write to tree
    write_file(project_base_path / "stage1.txt", "stage1");
    write_file(project_base_path / "stage2.txt", "stage2");
    fs::create_directory(project_base_path / "folder");
    write_file(project_base_path / "folder" / "stage3.txt", "stage3");

    std::string tree_hash = readProjectFolderAndWriteTree(project_base_path, true);
    /* std::cout << "Index hash: " << tree_hash << std::endl; */
    /* std::cout << "Old Tree Listing" << std::endl; */
    /* printTree(git_path,tree_hash); */
    /* write_file(project_base_path / ".cpp-git" / "HEAD", tree_hash); */

    // add new folder under folder
    fs::create_directory(project_base_path / "folder" / "new_folder");
    write_file(project_base_path / "folder" / "new_folder" / "new.txt", "new");

    std::string new_tree_hash = git_add_folder(project_base_path / "folder"/ "new_folder");
    GitTree* folder = findProjectFolderFromTree(new_tree_hash, "folder/new_folder", git_path);
    std::cout << "new_folder listing: " << std::endl;
    printer(folder->directory);

    fs::remove_all(project_base_path);
}
#endif

TEST(Staging, blank_index_read){
    std::string folder_name = "blank_index_read";
    git_folder_setup(folder_name);
    fs::path project_base_path = repo_find(fs::current_path() / folder_name);
    std::cout << "Project base path: " << project_base_path << std::endl;
    fs::path git_path = project_base_path / ".cpp-git";

    // See if return ptr is nullptr
    GitTree* tree_obj = getIndexTree(git_path);
    if (!tree_obj){
        std::cout << "YUP, tree_obj is null" << std::endl;
    }
    else{
        std::cout << "NOPE, tree_obj should have been null" << std::endl;
    }
    ASSERT_EQ(tree_obj,nullptr);
}

TEST(Staging, git_add_folder_blank_index){
    fs::path folder_name = "add_folder_blank_index";
    git_folder_setup(folder_name);
    fs::path project_base_path = repo_find(fs::current_path() / folder_name);
    std::cout << "Project base path: " << project_base_path << std::endl;
    fs::path git_path = project_base_path / ".cpp-git";

    // Create new files
    write_file(project_base_path / "stage1.txt", "stage1");
    write_file(project_base_path / "stage2.txt", "stage2");

    std::string tree_hash = git_add_folder(project_base_path);
    std::string index_hash = read_file(project_base_path / ".cpp-git" / "index");
    ASSERT_EQ(tree_hash,index_hash);
}

TEST(Staging, git_add_file_blank_index){
    fs::path folder_name = "add_file_blank_index";
    git_folder_setup(folder_name);
    fs::path project_base_path = repo_find(fs::current_path() / folder_name);
    std::cout << "Project base path: " << project_base_path << std::endl;
    fs::path git_path = project_base_path / ".cpp-git";

    // Create new file
    write_file(project_base_path / "stage1.txt", "stage1");

    std::string tree_hash = git_add_file(project_base_path / "stage1.txt");
    std::string index_hash = read_file(project_base_path / ".cpp-git" / "index");
    ASSERT_EQ(tree_hash,index_hash);
}

TEST(Staging, git_add_file_blankIndexCompareWithHEAD){
    fs::path folder_name = "add_file_blank_index";
    git_folder_setup(folder_name);
    fs::path project_base_path = repo_find(fs::current_path() / folder_name);
    std::cout << "Project base path: " << project_base_path << std::endl;
    fs::path git_path = project_base_path / ".cpp-git";

    // Create new files
    write_file(project_base_path / "stage1.txt", "stage1");
    write_file(project_base_path / "stage2.txt", "stage2");
    fs::create_directory(project_base_path / "folder");
    write_file(project_base_path /"folder" /"stage3.txt", "stage3");

    //write tree and write to master
    std::string tree_hash = readProjectFolderAndWriteTree(project_base_path);
    write_file(git_path / "refs" / "heads" / "master", tree_hash);

    // create new file in subfolder, and add to index
    write_file(project_base_path /"folder" /"stage4.txt", "stage4");
    std::string new_tree_hash = git_add_file(project_base_path/ "folder" /"stage4.txt");
    std::string index_hash = read_file(project_base_path / ".cpp-git" / "index");
    ASSERT_EQ(new_tree_hash,index_hash);
}


TEST(Status, path_relative_to_project){
    auto base_path = fs::current_path();
    auto project_file_path = fs::current_path() / "a" / "b";

    auto rel_path = path_relative_to_project(base_path,project_file_path);
    std::cout << "Relative Path: " << rel_path << std::endl;
    ASSERT_EQ(rel_path, "a/b");
}

TEST(Status, working_vs_index_multipleLevelNewFile){
    std::string folder_name = "working_vs_index_vs_working";
    git_folder_setup(folder_name);
    fs::path project_base_path = repo_find(fs::current_path() / folder_name);
    std::cout << "Project base path: " << project_base_path << std::endl;
    fs::path git_path = project_base_path / ".cpp-git";


    // Create new files and create tree object
    write_file(project_base_path / "stage1.txt", "stage1");
    write_file(project_base_path / "stage2.txt", "stage2");
    fs::create_directory(project_base_path / "folder");
    write_file(project_base_path /"folder" /"stage3.txt", "stage3");
    //write tree and write to index
    std::string tree_hash = readProjectFolderAndWriteTree(project_base_path,true);
    
    // create new file in subfolder and see untracked/modified file
    write_file(project_base_path /"folder" /"stage4.txt", "stage4");
    std::cout << "should say folder/stage4.txt is unstaged" << std::endl;
    git_status_index_vs_project(git_path);
}

TEST(Status, commit_vs_index_oneLevelNewFile){
    std::string folder_name = "commit_vs_index_oneLevelNewFile";
    git_folder_setup(folder_name);
    fs::path project_base_path = repo_find(fs::current_path() / folder_name);
    std::cout << "Project base path: " << project_base_path << std::endl;
    fs::path git_path = project_base_path / ".cpp-git";

    // Create new files and create tree object
    write_file(project_base_path / "stage1.txt", "stage1");
    write_file(project_base_path / "stage2.txt", "stage2");
    std::string tree_hash = readProjectFolderAndWriteTree(project_base_path);
    // write to HEAD
    write_file(project_base_path / ".cpp-git" / "refs" / "heads" / "master", tree_hash);

    // Add New File -> add to index
    write_file(project_base_path / "stage3.txt", "stage3");
    std::string index_hash = readProjectFolderAndWriteTree(project_base_path,true);

    std::cout << "Should say stage 3 is staged" << std::endl;
    git_status_commit_index(git_path);
}

TEST(Status, commit_vs_index_multipleLevelNewFile){
    std::string folder_name = "commit_vs_index_multipleLevelNewFile";
    git_folder_setup(folder_name);
    fs::path project_base_path = repo_find(fs::current_path() / folder_name);
    std::cout << "Project base path: " << project_base_path << std::endl;
    fs::path git_path = project_base_path / ".cpp-git";

    // Create new files and create tree object
    write_file(project_base_path / "stage1.txt", "stage1");
    write_file(project_base_path / "stage2.txt", "stage2");
    fs::create_directory(project_base_path / "folder");
    write_file(project_base_path /"folder" /"stage3.txt", "stage3");

    //write tree and write to head
    std::string tree_hash = readProjectFolderAndWriteTree(project_base_path);
    write_file(project_base_path / ".cpp-git" / "refs" / "heads" / "master", tree_hash);
    
    // create new file in subfolder, and add to index
    write_file(project_base_path /"folder" /"stage4.txt", "stage4");
    git_add_file(project_base_path/"folder" /"stage4.txt");

    std::cout << "Should say stage 4 is staged" << std::endl;
    // See new/modified file
    git_status_commit_index(git_path);
}

/* TEST(Status, commit_vs_index_multipleLevelDeletedFile){ */
/*     std::string folder_name = "commit_vs_index_multipleLevelDeletedFile"; */
/*     git_folder_setup(folder_name); */
/*     fs::path project_base_path = repo_find(fs::current_path() / folder_name); */
/*     std::cout << "Project base path: " << project_base_path << std::endl; */
/*     fs::path git_path = project_base_path / ".cpp-git"; */

/*     // Create new files and create tree object */
/*     write_file(project_base_path / "stage1.txt", "stage1"); */
/*     write_file(project_base_path / "stage2.txt", "stage2"); */
/*     fs::create_directory(project_base_path / "folder"); */
/*     write_file(project_base_path /"folder" /"stage3.txt", "stage3"); */

/*     //write tree and write to head */
/*     std::string tree_hash = readProjectFolderAndWriteTree(project_base_path); */
/*     write_file(project_base_path / ".cpp-git" / "refs" / "heads" / "master", tree_hash); */

/*     fs::remove(project_base_path / "folder" / "stage3.txt"); */
/* } */

/* ********* Git Init	********* */
// Test throw if not empty path
