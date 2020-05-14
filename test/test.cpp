#include <gtest/gtest.h>

#include <commands.hpp>
#include <commands.cpp>
#include <helper.cpp>
#include <helper.hpp>
#include <git_objects.hpp>
#include <git_objects.cpp>
#include <iostream>
#include <vector>

/* using namespace ranges; */
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
        std::cout << entry.path() << std::endl;
    }

    fs::remove_all(folder_path);
}

/* TEST(Scratch, basic_range) { */
/*     auto new_range = "ab\ncd" | views::split('\n'); */
/*     std::cout << new_range << std::endl; */
/* } */
/* TEST(Scratch, nested_range) { */
/*     auto test_range = "ab cd ef\ngh ij kl" | views::split('\n'); */
/*     std::cout << test_range << std::endl; */
/*     for (auto x : test_range) { */
/*         auto x_parts = x | views::split(' '); */
/*         /1* for (auto s : x_parts) { *1/ */
/*             /1* auto cnt = ranges::count_if(s, [](const auto& s) {return true;} ); *1/ */
/*             /1* std::cout << cnt << std::endl; *1/ */
/*         /1* } *1/ */
/*         std::cout << "New range: " << x_parts << std::endl; */
/*     } */
/* } */
/* TEST(Scratch, enumerate_range_seeWhichComesFirst) { */
/*     auto test_range = "ab cd ef\ngh ij kl" | views::split('\n'); */
/*     /1* std::cout << test_range << std::endl; *1/ */
/*     for (auto [idx, part] : views::enumerate(test_range)) { */
/*         std::cout << "Idx: " << idx << "Part: " << part << std::endl; */
/*     } */
/* } */

/* TEST(Scratch, nest_vector_range) { */
/*     std::string null_string = "ab cd ef\ngh ij kl"; */
/*     std::cout << "Length of string: " << null_string.length() << std::endl; */
/*     std::vector<char> no_null(null_string.begin(), null_string.end()); */
/*     std::cout << "Length of vector:" << no_null.size() << std::endl; */

/*     auto test_range = no_null | views::split('\n'); */
/*     std::cout << test_range << std::endl; */
/*     for (auto x : test_range) { */
/*         auto x_parts = x | views::split(' '); */
/*         std::cout << "New range: " << x_parts << std::endl; */
/*     } */
/* } */

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

TEST_F(GitTreeTest, write_object_and_read_object) {
    // check that writing and then reading an object
    // is still the same
    std::string message = "blob apple.txt dfa11\nblob orange.txt k12aq";
    std::cout << "Original Data Length: " << message.length() << std::endl;
    std::cout << message << std::endl;

    GitTree tree_obj(git_path, message);
    std::string hash = write_object(&tree_obj);
    GitTree read_result;
    read_into_object(read_result,git_path, hash);
    ASSERT_EQ(hash, write_object(&read_result, false));
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
    fs::path project_base_path;
    fs::path git_path;
    GitTree tree_obj;
    void SetUp() override {
        // Set up the repo
        git_folder_setup("TreeTraversal");
        project_base_path = repo_find(fs::current_path() / "TreeTraversal");
        std::cout << "Project base path: " << project_base_path << std::endl;
        git_path = project_base_path / ".cpp-git";

        // Create a tree
        message_1 = "file1";
        message_2 = "file2";
        GitBlob file_1(git_path, message_1);
        GitBlob file_2(git_path, message_2);
        /* std::cout << "File 1:" << std::endl; */
        std::string hash_1 = write_object(&file_1);
        /* std::cout << "File 2:" << std::endl; */
        std::string hash_2 = write_object(&file_2);
        tree_obj = GitTree(git_path);
        tree_obj.add_entry("blob", "file1.txt", hash_1);
        tree_obj.add_entry("blob", "file2.txt", hash_2);
        /* std::cout << "Tree: " << std::endl; */
        tree_hash = write_object(&tree_obj);
    }
};

TEST_F(TreeTraversal, one_level_tree) {
    /* // Now walk */
    walk_tree_and_replace(project_base_path,&tree_obj);

    // Check that we have written to project's path and that content is the same
    std::string final_message_1 = read_file(project_base_path / "file1.txt");
    /* final_message_1.pop_back(); */
    std::string final_message_2 = read_file(project_base_path / "file2.txt");
    /* final_message_2.pop_back(); */
    ASSERT_EQ(message_1, final_message_1);
    ASSERT_EQ(message_2, final_message_2);
}

TEST_F(TreeTraversal, two_level_tree) {
    // Create root tree
    std::string message_3 = "file3";
    GitBlob file_3(git_path, message_3);
    std::string hash_3 = write_object(&file_3);
    GitTree root_tree_obj(git_path);
    // Take tree from above and append to this tree
    root_tree_obj.add_entry("blob", "file3.txt", hash_3);
    root_tree_obj.add_entry("tree", "folder", tree_hash);
    std::string root_hash = write_object(&root_tree_obj);

    // Now walk from root
    walk_tree_and_replace(project_base_path,&root_tree_obj);

    // Check that everything is the same
    ASSERT_EQ(message_3, read_file(project_base_path / "file3.txt"));
    std::string final_message_1 = read_file(project_base_path / "folder" / "file1.txt");
    std::string final_message_2 = read_file(project_base_path / "folder" / "file2.txt");
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
    std::string stage_tree_hash = read_project_folder_and_write_tree(project_base_path, true);
    std::string index_hash = read_file(project_base_path / ".cpp-git" / "index");
    ASSERT_EQ(index_hash, stage_tree_hash);
}
#endif

TEST(Staging, git_add_file_oneLevelOneModifiedFile) {
    git_folder_setup("oneLevelOneModifiedFile");
    fs::path project_base_path = repo_find(fs::current_path() / "oneLevelOneModifiedFile");
    std::cout << "Project base path: " << project_base_path << std::endl;
    fs::path git_path = project_base_path / ".cpp-git";

    // Create a folder + files -> write to tree
    write_file(project_base_path / "stage1.txt", "stage1");
    write_file(project_base_path / "stage2.txt", "stage2");

    std::string tree_hash = read_project_folder_and_write_tree(project_base_path, true);
    std::cout << "Old Tree Hash" << std::endl;
    print_tree(git_path, tree_hash);
    /* write_file(project_base_path / ".cpp-git" / "HEAD", tree_hash); */
    // Modify stage2.txt and then add to index
    write_file(project_base_path / "stage2.txt", "changed stage2");
    std::string new_tree_hash = git_add_file(project_base_path / "stage2.txt");
    // compare the two tree objects
    std::cout << "New Tree Hash" << std::endl;
    print_tree(git_path, new_tree_hash);

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

    std::string tree_hash = read_project_folder_and_write_tree(project_base_path, true);
    std::cout << "Old Tree Listing" << std::endl;
    print_tree(git_path, tree_hash);
    /* write_file(project_base_path / ".cpp-git" / "HEAD", tree_hash); */
    // Modify stage2.txt and then add to index
    write_file(project_base_path / "stage3.txt", "stage3");
    std::string new_tree_hash = git_add_file(project_base_path / "stage3.txt");
    // compare the two tree objects
    std::cout << "New Tree Listing" << std::endl;
    print_tree(git_path, new_tree_hash);
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

    std::string tree_hash = read_project_folder_and_write_tree(project_base_path, true);
    /* std::cout << "Index hash: " << tree_hash << std::endl; */
    /* std::cout << "Old Tree Listing" << std::endl; */
    /* print_tree(git_path,tree_hash); */
    /* write_file(project_base_path / ".cpp-git" / "HEAD", tree_hash); */

    // Add new file under folder
    write_file(project_base_path / "folder" / "stage4.txt", "stage4");
    std::string new_tree_hash = git_add_file(project_base_path / "folder" / "stage4.txt");
    std::cout << "add_file new_tree_hash: " << new_tree_hash << std::endl;
    // compare the two tree objects
    /* std::cout << "New Tree Listing" << std::endl; */
    /* print_tree(git_path,new_tree_hash); */
    GitBlob* stage4_blob = find_project_file_from_tree_hash(new_tree_hash, "folder/stage4.txt", git_path);
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

    std::string tree_hash = read_project_folder_and_write_tree(project_base_path, true);
    /* std::cout << "Index hash: " << tree_hash << std::endl; */
    /* std::cout << "Old Tree Listing" << std::endl; */
    /* print_tree(git_path,tree_hash); */
    /* write_file(project_base_path / ".cpp-git" / "HEAD", tree_hash); */

    // Add new file under folder
    write_file(project_base_path / "folder" / "stage4.txt", "stage4");
    std::string new_tree_hash = git_add_folder(project_base_path / "folder");
    GitBlob* stage4_blob = find_project_file_from_tree_hash(new_tree_hash, "folder/stage4.txt", git_path);
    ASSERT_EQ(stage4_blob->data, "stage4");
    GitTree* folder = find_project_folder_from_tree(new_tree_hash, "folder", git_path);
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

    std::string tree_hash = read_project_folder_and_write_tree(project_base_path, true);
    /* std::cout << "Index hash: " << tree_hash << std::endl; */
    /* std::cout << "Old Tree Listing" << std::endl; */
    /* print_tree(git_path,tree_hash); */
    /* write_file(project_base_path / ".cpp-git" / "HEAD", tree_hash); */

    // add new folder under folder
    fs::create_directory(project_base_path / "folder" / "new_folder");
    write_file(project_base_path / "folder" / "new_folder" / "new.txt", "new");

    std::string new_tree_hash = git_add_folder(project_base_path / "folder");
    GitTree* folder = find_project_folder_from_tree(new_tree_hash, "folder/new_folder", git_path);
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
    GitTree* tree_obj = get_index_tree(git_path);
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
    // Maually add the folder ourselves to check with git add
    std::string tree_hash = read_project_folder_and_write_tree(project_base_path);

    git_add_folder(project_base_path);
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
    // Maually add the folder ourselves to check with git add
    std::string tree_hash = read_project_folder_and_write_tree(project_base_path);

    git_add_file(project_base_path / "stage1.txt");
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

    //write tree , write commit, and write to master
    std::string tree_hash = read_project_folder_and_write_tree(project_base_path);
    GitCommit commit_obj(git_path,tree_hash,"","first commit");
    std::string commit_hash = write_object(&commit_obj);
    write_file(git_path / get_current_branch_full(git_path), commit_hash);

    // create new file in subfolder, and add to index
    write_file(project_base_path /"folder" /"stage4.txt", "stage4");
    std::string new_tree_hash = git_add_file(project_base_path/ "folder" /"stage4.txt");
    std::string index_hash = read_file(project_base_path / ".cpp-git" / "index");

    // check that git add file was based off HEAD
    GitBlob* stage1_blob = find_project_file_from_tree_hash(new_tree_hash,"stage1.txt",git_path);
    ASSERT_EQ(stage1_blob->data,"stage1");
}

TEST(Staging, git_add_folder_blankIndexCompareWithHEAD){
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

    //write tree , write commit, and write to master
    std::string tree_hash = read_project_folder_and_write_tree(project_base_path);
    GitCommit commit_obj(git_path,tree_hash,"","first commit");
    std::string commit_hash = write_object(&commit_obj);
    write_file(git_path / get_current_branch_full(git_path), commit_hash);

    // create new file in subfolder, and add to index
    write_file(project_base_path /"folder" /"stage4.txt", "stage4");
    std::string new_tree_hash = git_add_folder(project_base_path/ "folder");
    std::string index_hash = read_file(project_base_path / ".cpp-git" / "index");

    // check that git add file was based off HEAD
    GitBlob* stage4_blob = find_project_file_from_tree_hash(new_tree_hash,"folder/stage4.txt",git_path);
    ASSERT_EQ(stage4_blob->data,"stage4");
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
    fs::current_path(project_base_path);

    // Create new files and create tree object
    write_file(project_base_path / "stage1.txt", "stage1");
    write_file(project_base_path / "stage2.txt", "stage2");
    fs::create_directory(project_base_path / "folder");
    write_file(project_base_path /"folder" /"stage3.txt", "stage3");
    //write tree and write to index
    std::string tree_hash = read_project_folder_and_write_tree(project_base_path,true);
    
    // create new file in subfolder and see untracked/modified file
    write_file(project_base_path /"folder" /"stage4.txt", "stage4");
    std::cout << "should say folder/stage4.txt is unstaged" << std::endl;
    git_status_index_vs_project();
    fs::current_path("..");
}

TEST(Status, commit_vs_index_oneLevelNewFile){
    std::string folder_name = "commit_vs_index_oneLevelNewFile";
    git_folder_setup(folder_name);
    fs::path project_base_path = repo_find(fs::current_path() / folder_name);
    std::cout << "Project base path: " << project_base_path << std::endl;
    fs::path git_path = project_base_path / ".cpp-git";
    fs::current_path(project_base_path);
    // Create new files
    write_file(project_base_path / "stage1.txt", "stage1");
    write_file(project_base_path / "stage2.txt", "stage2");
    
    //write tree , write commit, and write to master
    std::string tree_hash = read_project_folder_and_write_tree(project_base_path);
    GitCommit commit_obj(git_path,tree_hash,"","first commit");
    std::string commit_hash = write_object(&commit_obj);
    write_file(git_path / get_current_branch_full(git_path), commit_hash);

    // Add New File -> add to index
    write_file(project_base_path / "stage3.txt", "stage3");
    std::string index_hash = read_project_folder_and_write_tree(project_base_path,true);

    std::cout << "Should say stage 3 is staged" << std::endl;
    git_status_commit_index();
    fs::current_path("..");
}

TEST(Status, commit_vs_index_multipleLevelNewFile){
    std::string folder_name = "commit_vs_index_multipleLevelNewFile";
    git_folder_setup(folder_name);
    fs::path project_base_path = repo_find(fs::current_path() / folder_name);
    std::cout << "Project base path: " << project_base_path << std::endl;
    fs::path git_path = project_base_path / ".cpp-git";
    fs::current_path(project_base_path);

    // Create new files and create tree object
    try{
        write_file(project_base_path / "stage1.txt", "stage1");
        write_file(project_base_path / "stage2.txt", "stage2");
        fs::create_directory(project_base_path / "folder");
        write_file(project_base_path /"folder" /"stage3.txt", "stage3");

        //write tree , write commit, and write to master
        std::string tree_hash = read_project_folder_and_write_tree(project_base_path);
        GitCommit commit_obj(git_path,tree_hash,"","first commit");
        std::string commit_hash = write_object(&commit_obj);
        write_file(git_path / get_current_branch_full(git_path), commit_hash);
        
        // create new file in subfolder, and add to index
        write_file(project_base_path /"folder" /"stage4.txt", "stage4");
        git_add_file(project_base_path/"folder" /"stage4.txt");

        std::cout << "Should say stage 4 is staged" << std::endl;
        // See new/modified file
        git_status_commit_index();
    }
    catch (const char *e)
    {
        std::cout << e << std::endl;
        fs::current_path("..");
        throw e;
    }
    fs::current_path("..");
}

TEST(Status, commit_vs_index_oneLevelDeletedFile){
    std::string folder_name = "oneLevelDeletedFile";
    git_folder_setup(folder_name);
    fs::path project_base_path = repo_find(fs::current_path() / folder_name);
    std::cout << "Project base path: " << project_base_path << std::endl;
    fs::path git_path = project_base_path / ".cpp-git";
    fs::current_path(project_base_path);

    // Create new files and create tree object
    write_file(project_base_path / "stage1.txt", "stage1");
    write_file(project_base_path / "stage2.txt", "stage2");
    fs::create_directory(project_base_path / "folder");
    write_file(project_base_path /"folder" /"stage3.txt", "stage3");

    //write tree , write commit, and write to master
    std::string tree_hash = read_project_folder_and_write_tree(project_base_path);
    GitCommit commit_obj(git_path,tree_hash,"","first commit");
    std::string commit_hash = write_object(&commit_obj);
    write_file(git_path / get_current_branch_full(git_path), commit_hash);

    // Delete a file and add to index. 
    fs::remove(project_base_path / "stage2.txt");
    git_add_folder(project_base_path );

    std::cout << "Should say stage 2 is deleted" << std::endl;
    git_status_commit_index();
    fs::current_path("..");
}

TEST(Status, commit_vs_index_multipleLevelDeletedFileButNonEmptyFolder){
    std::string folder_name = "commit_vs_index_multipleLevelDeletedFileBuNonEmptyFolder";
    git_folder_setup(folder_name);
    fs::path project_base_path = repo_find(fs::current_path() / folder_name);
    std::cout << "Project base path: " << project_base_path << std::endl;
    fs::path git_path = project_base_path / ".cpp-git";
    fs::current_path(project_base_path);

    try{
        // Create new files and create tree object
        write_file(project_base_path / "stage1.txt", "stage1");
        write_file(project_base_path / "stage2.txt", "stage2");
        fs::create_directory(project_base_path / "folder");
        write_file(project_base_path /"folder" /"stage3.txt", "stage3");
        write_file(project_base_path /"folder" /"stage4.txt", "stage4");

        //write tree , write commit, and write to master
        std::string tree_hash = read_project_folder_and_write_tree(project_base_path);
        GitCommit commit_obj(git_path,tree_hash,"","first commit");
        std::string commit_hash = write_object(&commit_obj);
        write_file(git_path / get_current_branch_full(git_path), commit_hash);

        // Delete a file and add to index. 
        // NOTE this means folder is now empty
        fs::remove(project_base_path / "folder" / "stage3.txt");
        git_add_folder(project_base_path / "folder");

        std::cout << "Should say stage 3 is deleted" << std::endl;
        git_status_commit_index();
    }
    catch (char const *e){
        std::cout << e << std::endl;
        fs::current_path("..");
        throw e;
    }
    fs::current_path("..");
}
TEST(Status, commit_vs_index_multipleLevelDeletedFile){
    std::string folder_name = "commit_vs_index_multipleLevelDeletedFile";
    git_folder_setup(folder_name);
    fs::path project_base_path = repo_find(fs::current_path() / folder_name);
    std::cout << "Project base path: " << project_base_path << std::endl;
    fs::path git_path = project_base_path / ".cpp-git";
    fs::current_path(project_base_path);

    try{
        // Create new files and create tree object
        write_file(project_base_path / "stage1.txt", "stage1");
        write_file(project_base_path / "stage2.txt", "stage2");
        fs::create_directory(project_base_path / "folder");
        write_file(project_base_path /"folder" /"stage3.txt", "stage3");

        //write tree , write commit, and write to master
        std::string tree_hash = read_project_folder_and_write_tree(project_base_path);
        GitCommit commit_obj(git_path,tree_hash,"","first commit");
        std::string commit_hash = write_object(&commit_obj);
        write_file(git_path / get_current_branch_full(git_path), commit_hash);

        // Delete a file and add to index. 
        // NOTE this means folder is now empty
        fs::remove(project_base_path / "folder" / "stage3.txt");
        git_add_folder(project_base_path / "folder");

        std::cout << "Should say stage 3 is deleted" << std::endl;
        git_status_commit_index();
    }
    catch (char const *e){
        std::cout << e << std::endl;
        fs::current_path("..");
        throw e;
    }
    fs::current_path("..");
}


TEST(Status, commit_vs_index_multipleLevelModifiedFile){
    std::string folder_name = "commit_vs_index_multipleLevelModifiedFile";
    git_folder_setup(folder_name);
    fs::path project_base_path = repo_find(fs::current_path() / folder_name);
    std::cout << "Project base path: " << project_base_path << std::endl;
    fs::path git_path = project_base_path / ".cpp-git";
    fs::current_path(project_base_path);

    // Create new files and create tree object
    write_file(project_base_path / "stage1.txt", "stage1");
    write_file(project_base_path / "stage2.txt", "stage2");
    fs::create_directory(project_base_path / "folder");
    write_file(project_base_path /"folder" /"stage3.txt", "stage3");

    //write tree , write commit, and write to master
    std::string tree_hash = read_project_folder_and_write_tree(project_base_path);
    GitCommit commit_obj(git_path,tree_hash,"","first commit");
    std::string commit_hash = write_object(&commit_obj);
    write_file(git_path / get_current_branch_full(git_path), commit_hash);

    // Modify a file and add to index. 
    // NOTE this means folder is now empty
    write_file(project_base_path / "folder" / "stage3.txt", "changed");
    git_add_folder(project_base_path / "folder");

    std::cout << "Should say stage 3 is modified" << std::endl;
    git_status_commit_index();
    fs::current_path("..");

}

TEST(Status, everything){
    std::string folder_name = "commit_vs_index_multipleLevelModifiedFile";
    git_folder_setup(folder_name);
    fs::path project_base_path = repo_find(fs::current_path() / folder_name);
    std::cout << "Project base path: " << project_base_path << std::endl;
    fs::path git_path = project_base_path / ".cpp-git";

    fs::current_path(project_base_path);

    // Create new files and create tree object
    write_file(project_base_path / "stage1.txt", "stage1");
    write_file(project_base_path / "stage2.txt", "stage2");
    fs::create_directory(project_base_path / "folder");
    write_file(project_base_path /"folder" /"stage3.txt", "stage3");

    //write tree , write commit, and write to master
    std::string tree_hash = read_project_folder_and_write_tree(project_base_path);
    GitCommit commit_obj(git_path,tree_hash,"","first commit");
    std::string commit_hash = write_object(&commit_obj);
    write_file(git_path / get_current_branch_full(git_path), commit_hash);

    // Modify, delete, and create -> then add to index. 
    write_file(project_base_path / "folder" / "stage3.txt", "changed");
    write_file(project_base_path / "folder" / "stage4.txt", "stage4");
    fs::remove(project_base_path / "stage2.txt");
    git_add_folder(project_base_path );

    std::cout << "Should say stage2 is deleted, stage3 is changed, stage4 is new" << std::endl;
    git_status_commit_index();
    git_status_index_vs_project();

    fs::current_path("..");
}

TEST(Checkout, file){
    std::string folder_name = "checkout_file";
    git_folder_setup(folder_name);
    fs::path project_base_path = repo_find(fs::current_path() / folder_name);
    std::cout << "Project base path: " << project_base_path << std::endl;
    fs::path git_path = project_base_path / ".cpp-git";
    fs::current_path(project_base_path);

    // Create new files and create tree object
    write_file(project_base_path / "stage1.txt", "stage1");
    write_file(project_base_path / "stage2.txt", "stage2");
    fs::create_directory(project_base_path / "folder");
    write_file(project_base_path /"folder" /"stage3.txt", "stage3");
    // git add and git commit
    git_add_folder(project_base_path);
    git_commit("first commit");

    // change a file
    write_file(project_base_path / "folder" / "stage3.txt", "changed");
    // checkout previous version
    git_checkout_file(project_base_path / "folder" / "stage3.txt");
    ASSERT_EQ(read_file(project_base_path/"folder" / "stage3.txt"),"stage3");
    fs::current_path("..");
}

TEST(Checkout, branch){
    std::string folder_name = "checkout_file";
    git_folder_setup(folder_name);
    fs::path project_base_path = repo_find(fs::current_path() / folder_name);
    std::cout << "Project base path: " << project_base_path << std::endl;
    fs::path git_path = project_base_path / ".cpp-git";
    fs::current_path(project_base_path);

    // Create new files and create tree object
    write_file(project_base_path / "stage1.txt", "stage1");
    write_file(project_base_path / "stage2.txt", "stage2");
    fs::create_directory(project_base_path / "folder");
    write_file(project_base_path /"folder" /"stage3.txt", "stage3");
    // git add and git commit
    git_add_folder(project_base_path);
    git_commit("first commit");

    // change some files
    write_file(project_base_path / "stage2.txt", "changed");
    write_file(project_base_path / "folder" / "stage3.txt", "changed");
    // checkout previous version
    git_checkout_branch("master");
    ASSERT_EQ(read_file(project_base_path/"folder" / "stage3.txt"),"stage3");
    ASSERT_EQ(read_file(project_base_path/ "stage2.txt"),"stage2");
    fs::current_path("..");
}

/* ********* Git Init	********* */
// Test throw if not empty path


TEST(GitCommand, git_init){
    // make tmp folder
    fs::path worktree = fs::current_path() / "init_tmp";
    /* std::cout << "Folder path: " << folder_path << std::endl; */
    if (fs::exists(worktree)) {
        fs::remove_all(worktree);
    }
    fs::create_directory(worktree);
    fs::current_path(worktree);
    cmd_init({});
    fs::path git_path = worktree / ".cpp-git";
    ASSERT_TRUE(fs::exists(git_path / "HEAD"));
    ASSERT_TRUE(fs::exists(git_path / "objects"));
    ASSERT_TRUE(fs::exists(git_path / "branches"));
    ASSERT_TRUE(fs::exists(git_path / "index"));
    ASSERT_TRUE(fs::exists(git_path / "refs" / "tags"));
    ASSERT_TRUE(fs::exists(git_path / "refs" / "heads"));
    ASSERT_TRUE(fs::exists(git_path / "refs" / "heads" / "master"));
    ASSERT_EQ(read_file(git_path / "HEAD"), "ref: refs/heads/master");
    fs::current_path("..");
}

void add_testing_file(fs::path file_path, std::string content){
    write_file(file_path, content);
    cmd_add(std::vector<std::string>{file_path});
    std::cout<<"Finish adding a new file: "<< file_path <<std::endl;
}

TEST(GitCommand, git_commit){
    fs::path worktree = "test_git_commit";
    git_folder_setup(worktree);
    worktree = fs::canonical(worktree);
    fs::path git_path = worktree / ".cpp-git";
    //change working directory since git commit should be executed under a git repo
    fs::current_path(worktree);

    try{
        // git add a new file
        std::string filename = "test.txt", content = "Now test git_commit command";
        add_testing_file(worktree / filename, content);

        // A new commit,
        std::string message = "Initial commit";
        git_commit(message);
        
        std::cout<<"Finish invoking git commit"<<std::endl;
        // Check that index has been cleared
        ASSERT_EQ(read_file(git_path / "index"),"");
        // Get commit from HEAD and check it's content
        ASSERT_EQ(read_file(git_path / "HEAD"), "ref: refs/heads/master");
        std::string hash = ref_resolve(git_path / "HEAD");
        GitCommit commit_obj;
        read_into_object(commit_obj,git_path,hash);
        ASSERT_EQ(commit_obj.commit_message, message);
        std::cout<<"Finish testing content"<<std::endl;

        // Get tree node and check if there's a txt file with correct content
        GitTree tree_obj;
        read_into_object(tree_obj,git_path,commit_obj.tree_hash);
        ASSERT_EQ(tree_obj.directory.size(), 1);
        ASSERT_EQ(tree_obj.directory[0].name, filename);
        GitBlob blob_obj;
        read_into_object(blob_obj,git_path,tree_obj.directory[0].hash);
        ASSERT_EQ(blob_obj.data, content);
    }
    catch (char const *e)
    {
        std::cout << e << std::endl;
        throw e;
    }
    fs::current_path("..");
}

TEST(GitCommand, cmd_tag){
    fs::path worktree = "test_git_commit";
    git_folder_setup(worktree);
    worktree = fs::canonical(worktree);
    fs::path git_path = worktree / ".cpp-git";
    //change working directory since git commit should be executed under a git repo
    fs::current_path(worktree);

    try{
        cmd_tag({});
        std::cout << "-------" << std::endl;
        std::string filename = "test.txt", content = "Now test git_commit command";
        add_testing_file(worktree / filename, content);
        std::string message = "Initial commit";
        git_commit(message);
        cmd_tag({"v0.1"});
        cmd_tag({"-a", "v0.2", "-m", "hello world"});
        cmd_tag({"-a", "v0.3"});
        std::cout << "-------" << std::endl;
        cmd_tag({});

        std::string hash = ref_resolve(git_path / "refs" / "tags" / "v0.2");
        GitTag tag_obj;
        read_into_object(tag_obj,git_path,hash);
        std::string head_commit_hash = ref_resolve(git_path / "HEAD");
        ASSERT_EQ(tag_obj.tag_message, "hello world");
        ASSERT_EQ(tag_obj.commit_hash, head_commit_hash);

        hash = ref_resolve(git_path / "refs" / "tags" / "v0.3");
        GitTag tag_obj2;
        read_into_object(tag_obj2,git_path,hash);
        ASSERT_EQ(tag_obj2.tag_message, "");
        ASSERT_EQ(tag_obj2.commit_hash, head_commit_hash);
    }
    catch (char const *e)
    {
        std::cout << e << std::endl;
        throw e;
    }
    fs::current_path("..");
}

TEST(GitCommand, cmd_log){
    fs::path worktree = "test_git_commit";
    git_folder_setup(worktree);
    worktree = fs::canonical(worktree);
    fs::path git_path = worktree / ".cpp-git";
    //change working directory since git commit should be executed under a git repo
    fs::current_path(worktree);

    try{
        cmd_log({});

        std::string filename = "test.txt", content = "Now test git_commit command";
        add_testing_file(worktree / filename, content);
        std::string message = "Initial commit";
        git_commit(message);

//        cmd_log();
        filename = "test2.txt";
        add_testing_file(worktree / filename, content);
        message = "Second commit";
        git_commit(message);

        std::cout << "Should Output all commit:" << std::endl;
        cmd_log({});

        std::cout << "Should Output the second commit only:" << std::endl;
        cmd_log({"-n", "1"});
    }
    catch (char const *e)
    {
        std::cout << e << std::endl;
        throw e;
    }
    fs::current_path("..");
}

#if 0
TEST(GitCommand, git_reset){
    fs::path worktree = "test_git_reset";
    git_folder_setup(worktree);
    worktree = fs::canonical(worktree);
    fs::path git_path = worktree / ".cpp-git";
    fs::current_path(worktree);//change working directory
    auto get_HEAD_tree_hash = [&](){
        std::string head_commit_hash = ref_resolve(git_path / "HEAD");
        return dynamic_cast<GitCommit*>(read_object(git_path, head_commit_hash))->tree_hash;
    };
    try{
        // git add a new file
        std::string filename = "test.txt", content = "Now test git_rest command";
        add_testing_file(worktree / filename, content);

        // get the initial tree hash
        std::string HEAD_tree_hash = get_HEAD_tree_hash();
        // test git reset --mixed as default;
        ASSERT_NE(read_file(git_path / "index"), HEAD_tree_hash);
        cmd_reset(std::vector<std::string>{});
        ASSERT_EQ(read_file(git_path / "index"), HEAD_tree_hash);
        ASSERT_EQ(read_file(worktree / filename), content);

        // test git reset --hard
        //   commit to change HEAD
        add_testing_file(worktree / filename, content);
        cmd_commit(std::vector<std::string>{"-m", "Init commit"});
        HEAD_tree_hash = get_HEAD_tree_hash();
        //   Add modified file
        std::string new_content = "Now test git_rest command with '--hard' option";
        add_testing_file(worktree / filename, new_content);
        ASSERT_NE(read_file(git_path / "index"), HEAD_tree_hash);
        cmd_reset(std::vector<std::string>{"--hard"});
        //   test if index is reset to head and files are replaced
        ASSERT_EQ(read_file(git_path / "index"), HEAD_tree_hash);
        ASSERT_EQ(read_file(worktree / filename), content);
    }
    catch (char const *e)
    {
        std::cout << e << std::endl;
        throw e;
    }
    fs::current_path("..");
}
#endif
TEST(Branching, new_branch){
    std::string folder_name = "checkout_file";
    git_folder_setup(folder_name);
    fs::path project_base_path = repo_find(fs::current_path() / folder_name);
    std::cout << "Project base path: " << project_base_path << std::endl;
    fs::path git_path = project_base_path / ".cpp-git";
    fs::current_path(project_base_path);

    // Create new files
    write_file(project_base_path / "stage1.txt", "stage1");
    write_file(project_base_path / "stage2.txt", "stage2");
    fs::create_directory(project_base_path / "folder");
    write_file(project_base_path /"folder" /"stage3.txt", "stage3");
    // git add and git commit
    git_add_folder(project_base_path);
    git_commit("first commit");

    git_branch_new("new_branch");
    git_checkout_branch("new_branch");
    ASSERT_EQ(read_file(git_path/ "HEAD"),"ref: refs/heads/new_branch");
    fs::current_path("..");
}

TEST(Branching, branch_delete){
    std::string folder_name = "checkout_file";
    git_folder_setup(folder_name);
    fs::path project_base_path = repo_find(fs::current_path() / folder_name);
    std::cout << "Project base path: " << project_base_path << std::endl;
    fs::path git_path = project_base_path / ".cpp-git";
    fs::current_path(project_base_path);

    // Create new files
    write_file(project_base_path / "stage1.txt", "stage1");
    write_file(project_base_path / "stage2.txt", "stage2");
    fs::create_directory(project_base_path / "folder");
    write_file(project_base_path /"folder" /"stage3.txt", "stage3");
    // git add and git commit
    git_add_folder(project_base_path);
    git_commit("first commit");

    git_branch_new("new_branch");
    git_branch_delete("new_branch");
    try{
        git_checkout_branch("new_branch");
    }
    catch (char const* e){
        std::cout << e << std::endl;
    }
    fs::current_path("..");
}

TEST(Branching, branch_list){
    std::string folder_name = "checkout_file";
    git_folder_setup(folder_name);
    fs::path project_base_path = repo_find(fs::current_path() / folder_name);
    std::cout << "Project base path: " << project_base_path << std::endl;
    fs::path git_path = project_base_path / ".cpp-git";
    fs::current_path(project_base_path);

    git_branch_new("new_branch");
    git_branch_new("new_branch2");
    std::cout << "Should list master, new_branch, new_branch2" << std::endl;
    git_branch_list();
    fs::current_path("..");
}

TEST(Latest, check_find_error){
    std::string test_string = "nospaces\n";
    auto space_delimiter = [](auto x){return x==' ';};

    auto endpoint_it = std::find_if(test_string.cbegin(),test_string.cend(),space_delimiter);
    try{
        check_find_error(endpoint_it,test_string.end(),"tree");
    }
    catch (std::string& e){
        std::cout << e << std::endl;
    }
}

TEST(Latest, extract_one_tree_node_oneNode){
    std::string folder_name = "extract_one_tree_node_oneNode";
    git_folder_setup(folder_name);
    fs::path project_base_path = repo_find(fs::current_path() / folder_name);
    std::cout << "Project base path: " << project_base_path << std::endl;
    fs::path git_path = project_base_path / ".cpp-git";
    fs::current_path(project_base_path);

    GitTree tree_obj(git_path);
    tree_obj.add_entry("blob","file1","abhash");
    std::string tree_hash = write_object(&tree_obj);

    GitTree same_tree_obj;
    read_into_object(same_tree_obj,git_path,tree_hash);
    GitTreeNode node = same_tree_obj.directory[0];
    ASSERT_EQ(node.type,"blob");
    ASSERT_EQ(node.name,"file1");
    ASSERT_EQ(node.hash,"abhash");

}
TEST(Latest, extract_one_tree_node_twoNode){
    std::string folder_name = "extract_one_tree_node_twoNode";
    git_folder_setup(folder_name);
    fs::path project_base_path = repo_find(fs::current_path() / folder_name);
    std::cout << "Project base path: " << project_base_path << std::endl;
    fs::path git_path = project_base_path / ".cpp-git";
    fs::current_path(project_base_path);

    GitTree tree_obj(git_path);
    tree_obj.add_entry("blob","file1","abhash");
    tree_obj.add_entry("tree","tree1","cdhash");
    std::string tree_hash = write_object(&tree_obj);

    GitTree same_tree_obj;
    read_into_object(same_tree_obj,git_path,tree_hash);
    GitTreeNode node = same_tree_obj.directory[0];
    GitTreeNode node2 = same_tree_obj.directory[1];
    ASSERT_EQ(node.type,"blob");
    ASSERT_EQ(node.name,"file1");
    ASSERT_EQ(node.hash,"abhash");

    ASSERT_EQ(node2.type,"tree");
    ASSERT_EQ(node2.name,"tree1");
    ASSERT_EQ(node2.hash,"cdhash");
}
