#include "git_objects.hpp"
#include "helper.hpp"
#include "commands.hpp"

#include <string>
#include <filesystem>
#include <iostream>
#include <vector>
#include <unordered_map>
#include <set>
/* ********* TEMP	********* */

void walk_tree_and_replace(fs::path tree_write_path, GitObject* obj) {
    fs::path git_path = repo_find(tree_write_path) / ".cpp-git";

    GitTree* tree_obj = dynamic_cast<GitTree*>(obj);
    std::cout << "Tree listing: " << std::endl;
    printer(tree_obj->directory);
    for (auto node : tree_obj->directory) {
        if (node.type == "blob") {
            /* std::cout << "Write Path: "<< (tree_write_path / node.name) << std::endl; */
            write_object_to_project_file(tree_write_path / node.name, node.hash);
        } else if (node.type == "tree") {
            /* std::cout << "Got here" << std::endl; */
            GitObject* obj = read_object(git_path, node.hash);
            walk_tree_and_replace(tree_write_path / node.name, obj);
        } else {
            std::cout << "Node should only be tree or blob" << std::endl;
            throw "Node type should only be tree or blob";
        }
    }
}
/* ********* Traversal Algorithms	********* */

std::string get_subtree_hash_for_new_file(GitTree* tree_obj, typename fs::path::iterator file_it,
                                const typename fs::path::iterator end_it, const fs::path git_path,
                                const fs::path& file_path) {
    // New Tree Object we are creating
    GitTree new_tree_obj(git_path);

    /* std::cout << "Currently at: " << *file_it << std::endl; */
    bool found = false;
    for (auto node : tree_obj->directory) {
        // Case 1: Same branch as file
        if (check_node_name(node,*file_it)) {
            // SubCase 1: file_it is at last element
            bool end = end_of_path(file_it,end_it);
            if (end) {
                // create GitBlob object, write to .git, then add to tree obj
                std::string blob_hash = read_project_file_and_write_object(git_path, file_path);
                new_tree_obj.add_entry(node.type, node.name, blob_hash);
                found = true;
            }
            // SubCase 2 : still haven't reached file
            else {
                check_if_tree(node);
                GitTree* subtree = get_tree_from_hash(node.hash,git_path);
                auto new_it = file_it;
                std::string subtree_hash =
                    get_subtree_hash_for_new_file(subtree, ++new_it, end_it, git_path, file_path);
                new_tree_obj.add_entry("tree", node.name, subtree_hash);
            }
        }
        // Case 2: Not what we want to add to index
        else {
            new_tree_obj.add_entry(node.type, node.name, node.hash);
        }
    }
    // EC: if at end and we haven't found the file in the old tree directory
    std::cout << "Currently at: " << *file_it << std::endl;
    if (end_of_path(file_it,end_it) && !found) {
        std::string blob_hash = read_project_file_and_write_object(git_path, file_path);
        new_tree_obj.add_entry("blob", file_path.filename(), blob_hash);
    }
    // Write Tree and return hash
    std::cout << "Post Listing: " << std::endl;
    printer(new_tree_obj.directory);
    return write_object(&new_tree_obj);
}
std::string get_subtree_hash_for_new_folder(GitTree* tree_obj, typename fs::path::iterator file_it,
                                  typename fs::path::iterator end_it, const fs::path git_path,
                                  const fs::path folder_path) {
    // New Tree Object we are creating
    GitTree new_tree_obj(git_path);

    std::cout << "Pre: currently at " << *file_it << std::endl;
    bool found = false;
    for (auto node : tree_obj->directory) {
        // Case 1: Same branch as file
        if (check_node_name(node,*file_it)) {
            // SubCase 1: node points to adding folder
            bool end = end_of_path(file_it,end_it);
            if (end) {
                check_if_tree(node);
                std::string subtree_hash = read_project_folder_and_write_tree(folder_path);
                // add to new_tree
                new_tree_obj.add_entry(node.type, node.name, subtree_hash);
                found = true;
            }
            // SubCase 2 : still haven't reached folder
            else {
                check_if_tree(node);
                GitTree* subtree = get_tree_from_hash(node.hash,git_path);
                auto new_it = file_it;
                std::string subtree_hash =
                    get_subtree_hash_for_new_folder(subtree, ++new_it, end_it, git_path, folder_path);
                new_tree_obj.add_entry("tree", node.name, subtree_hash);
            }
        }
        // Case 2: Not what we want to add to index
        else {
            new_tree_obj.add_entry(node.type, node.name, node.hash);
        }
    }
    // EC: if at end and we haven't found the file in the old tree directory
    if (end_of_path(file_it,end_it) && !found) {
        /* std::cout << "Name of mistaken write: " << folder_path.filename() << std::endl; */
        std::string subtree_hash = read_project_folder_and_write_tree(folder_path);
        // add to new_tree
        new_tree_obj.add_entry("blob", folder_path.filename(), subtree_hash);
    }
    // Write Tree and return hash
    /* std::cout << "Post: currently at: " << *file_it << std::endl; */
    /* printer(new_tree_obj.directory); */
    return write_object(&new_tree_obj);
}
/* ********* 	********* */
int test_function(void) {
    std::vector<int> test;
    std::cout << "Hello World" << std::endl;
    return 0;
}


void cmd_init(const std::vector<std::string>& args){
    if ((args.size() >= 1 && args[0] == "--help") || args.size() > 1)
    {
        throw GIT_INIT_USAGE;
    }
    git_init(args[0]);
}


void cmd_add(const std::vector<std::string> &args){
    for(auto path: args){
        if (path != "/") path = fs::canonical(path);
        if (fs::is_directory(path)){
            std::string ERROR = "TODO: add git add folder";
        }
        else{
            git_add_file(path);
        }
    }
}

void cmd_cat_file(const std::vector<std::string> &args){
    if (args.size() <= 1 || CAT_FILE_SUBCMDS.find(args[0]) == CAT_FILE_SUBCMDS.end() || args[0] == "--help")
    {
        throw CAT_FILE_USAGE;
    }
    git_cat_file(fs::canonical(args[1]), args[0]);
}

void cmd_checkout(const std::vector<std::string>& args){
    if (args.size() != 1)
    {
        throw CHECKOUT_USAGE;
    }
    git_checkout(args[0]);
}

void cmd_commit(const std::vector<std::string>& args){
    if (args.size() != 2)
    {
        throw "Currently only support `git commit -m 'your commit message'`";
    }
    git_commit(args[1]);
}

void cmd_reset(const std::vector<std::string>& args){
    if (args.size() == 0 || args[0] == "--mixed")
        git_reset(false);
    else
        git_reset(true);
}

void git_cat_file(fs::path obj, const std::string& fmt){
    fs::path repo = repo_find(fs::current_path());
    GitObject* object = read_object(repo, object_find(repo, obj, fmt));
    std::cout << object->to_filesystem() << std::endl;
}


void git_init(fs::path project_base_path) {
    // check that .git doesn't exist or is empty directory
    fs::path git_path = project_base_path / ".cpp-git";
    if (fs::exists(git_path)) {
        throw "init error: Not an empty path";
    }
    // create object dir
    fs::create_directories(git_path / "objects");
    // create HEAD file with "ref: refs/heads/master"
    write_file(git_path / "HEAD", "ref: refs/heads/master");
    write_file(git_path / "index", "");
    write_file(git_path / "refs" / "heads" / "master","");

    // create branches dir
    fs::create_directories(git_path / "branches");
    // create refs dir with tags+heads subdirectory
    fs::create_directories(git_path / "refs" / "tags");
}

void git_checkout(std::string hash){
    auto repo_base_path = repo_find(fs::current_path());
    GitObject* obj = read_object(repo_base_path / ".cpp-git", hash);
    std::string fmt = obj->get_fmt();
    if (fmt == "commit") {
        GitCommit* commit_obj = dynamic_cast<GitCommit*>(obj);
        git_checkout(commit_obj->tree_hash);
    } else {
        throw "Shouldn't reach here";
    }
}

void git_commit(std::string commit_message){
    fs::path project_base_path = repo_find(fs::current_path());
    fs::path git_path = project_base_path / ".cpp-git";
    
    // Hash of the previous commit (aka HEAD), might be empty if it's the first commit
    std::string current_branch_name = get_current_branch(git_path);
    std::string current_commit_hash = get_commit_hash_from_branch(current_branch_name,git_path);

    // Hash of the new tree in index
    std::string index_tree_hash = get_tree_hash_of_index(git_path);

    GitCommit new_commit_obj = GitCommit(git_path,index_tree_hash,current_commit_hash,commit_message);
    std::string new_commit_hash = write_object(&new_commit_obj);
    write_file(git_path / current_branch_name, new_commit_hash);
    // clean index for the next round
    write_file(git_path / "index", "");
}

void git_reset(bool hard){
    fs::path worktree = repo_find(fs::current_path());
    fs::path git_path = worktree / ".cpp-git";
    // Get tree_hash from HEAD
    std::string head_commit_hash = ref_resolve(git_path / "HEAD");
    GitCommit* head_commit = dynamic_cast<GitCommit*>(read_object(git_path, head_commit_hash));
    std::string head_commit_tree_hash = head_commit->tree_hash;
    write_file(git_path / "index", head_commit_tree_hash);

    if (hard){
        GitTree* tree = dynamic_cast<GitTree*>(read_object(git_path, head_commit_tree_hash));
        walk_tree_and_replace(worktree, tree);
    }
}

// NOTE: can add new files, but they must be in existing folders already
std::string git_add_file(const fs::path& file_path) {
    fs::path project_base_path = repo_find(file_path);
    fs::path git_path = project_base_path / ".cpp-git";
    std::string new_tree_hash;

    // Run file_it
    auto base_it = project_base_path.begin();
    auto file_it = file_path.begin();
    while (base_it != project_base_path.end()) {
        base_it++;
        file_it++;
    }
    // *file_it=folder in /project_base_path/folder/etc...
    // Paths are unique, but individual names arn't
    // EC: if file is one_level
    // EC: if file is new file
    GitTree* index_tree = get_index_tree(git_path);
    if (!index_tree){
        // SubCase 1: if even head is empty, just add from project folder instead of traversing git trees
        GitTree* head_tree = get_head_tree(git_path);
        if (!head_tree){
            // TODO: check that folder path and project base path are the same
            std::string blob_hash = read_project_file_and_write_object(git_path,file_path);
            GitTree tree_obj(git_path);
            tree_obj.add_entry("blob",file_path.filename(),blob_hash);
            new_tree_hash = write_object(&tree_obj);
            write_file(git_path / "index",new_tree_hash);
        }
        else{
        // SubCase 2: then base off head tree instead
            new_tree_hash = get_subtree_hash_for_new_file(head_tree,file_it,file_path.end(),git_path,file_path);
        }
    }
    else{
        new_tree_hash =
            get_subtree_hash_for_new_file(index_tree, file_it, file_path.end(), git_path, file_path);
    }

    std::cout << "Should write to index now" << std::endl;
    write_file(git_path / "index", new_tree_hash);
    return new_tree_hash;
}


std::string git_add_folder(const fs::path folder_path) {
    fs::path project_base_path = repo_find(folder_path);
    fs::path git_path = project_base_path / ".cpp-git";
    std::string new_tree_hash;

    // Run file_it
    auto base_it = project_base_path.begin();
    auto file_it = folder_path.begin();
    while (base_it != project_base_path.end()) {
        base_it++;
        file_it++;
    }

    GitTree* index_tree = get_index_tree(git_path);
    if (!index_tree){
        GitTree* head_tree = get_head_tree(git_path);
        if (!head_tree){
            if (folder_path == project_base_path){
                new_tree_hash = read_project_folder_and_write_tree(folder_path);
            }
            else{
                throw "git add error. Add the parent directory first";
            }
        }
        else{
        // SubCase 2: then base off head tree instead
            new_tree_hash = get_subtree_hash_for_new_folder(head_tree,file_it,folder_path.end(),git_path,folder_path);
        }
    }
    else{
        new_tree_hash =
            get_subtree_hash_for_new_folder(index_tree, file_it, folder_path.end(), git_path, folder_path);
    }

    std::cout << "Should write to index now" << std::endl;
    write_file(git_path / "index", new_tree_hash);
    return new_tree_hash;
}

void get_project_file_hashes(fs::path directory,std::vector<std::string>& project_leaf_hashes,std::unordered_map<std::string,std::string>& path_to_hash_dict, const fs::path git_path){
    for (auto entry: fs::directory_iterator(directory)){
        fs::path entry_path = entry.path();
        if (fs::is_regular_file(entry_path)){
            std::string file_hash = read_project_file_and_write_object(git_path,entry_path);
            project_leaf_hashes.push_back(file_hash);
            path_to_hash_dict.insert({entry_path.string(),file_hash});
        }
        else if (fs::is_directory(entry_path)){
            get_project_file_hashes(entry_path,project_leaf_hashes,path_to_hash_dict,git_path);
        }
        else{
            throw "cpp-git cannot handle this file";
        }
    }
}



/* void print_new_hashes(fs::path directory, const std::set<std::string>& diff_hashes,const std::unordered_map<std::string,std::string>& path_to_hash_dict,const fs::path project_base_path){ */
/*     std::cout << "---------------Unstaged changes------------" << std::endl; */
/*     for (auto entry: fs::directory_iterator(directory)){ */
/*         fs::path entry_path = entry.path(); */
/*         auto cached_hash = path_to_hash_dict.at(entry_path.string()); */
/*         if (fs::is_regular_file(entry_path)){ */
/*             bool found = isInSet(diff_hashes,cached_hash); */
/*             if (found){ */
/*                 std::cout << "new/modified file: " << path_relative_to_project(project_base_path,entry_path) << std::endl; */
/*             } */
/*         } */
/*         else if (fs::is_directory(entry_path)){ */
/*             print_new_hashes(entry_path,diff_hashes,path_to_hash_dict,project_base_path); */
/*         } */
/*         else{ */
/*             throw "cpp-git cannot handle this file"; */
/*         } */
/*     } */
/* } */

std::set<std::string> set_difference_of_leaf_hashes(std::vector<std::string>& leaf_hashes_1, std::vector<std::string>& leaf_hashes_2){
    std::sort(leaf_hashes_1.begin(),leaf_hashes_1.end());
    std::sort(leaf_hashes_2.begin(),leaf_hashes_2.end());

    std::vector<std::string> diff_hashes;
    std::set_difference(leaf_hashes_1.begin(),leaf_hashes_1.end(),leaf_hashes_2.begin(),leaf_hashes_2.end(),std::back_inserter(diff_hashes));
    return std::set<std::string>(diff_hashes.begin(),diff_hashes.end());
}

/* ********* 	********* */
void get_leaf_hashes_of_tree(GitTree* tree_obj,std::set<std::string>& index_leaf_hashes, const fs::path git_path){
    for (auto node: tree_obj->directory){
        if (node.type=="blob"){
            index_leaf_hashes.insert(node.hash);
        }
        else if (node.type == "tree"){
            GitTree* subtree = get_tree_from_hash(node.hash,git_path);
            get_leaf_hashes_of_tree(subtree,index_leaf_hashes,git_path);
        }
        else{
            throw "cpp-git cannot handle this file";
        }
    }
}

void print_unstaged_project_files(fs::path directory, const std::set<std::string>& index_leaf_hashes, const fs::path git_path, const fs::path project_base_path){
    for(auto project_file: fs::directory_iterator(directory)){
        fs::path project_file_path = project_file.path();
        /* std::cout << "file path: " << project_file_path << std::endl; */
        if(is_git_repo(project_file_path)){
            continue;
        }

        if (fs::is_regular_file(project_file_path)){
            std::string file_hash = read_project_file_and_write_object(git_path,project_file_path);
            if (!is_in_set(index_leaf_hashes,file_hash)){
                std::cout << "new/modified: " <<  path_relative_to_project(project_base_path,project_file_path) << std::endl;
            }
        }
        else if (fs::is_directory(project_file_path)){
            print_unstaged_project_files(project_file_path,index_leaf_hashes,git_path,project_base_path);
        }
        else{
            throw "cpp-git cannot handle this file";
        }
    }
}

void print_new_index_nodes_and_calc_delete(GitTree* index_tree,std::set<std::string>& delete_hashes, const std::set<std::string>& head_leaf_hashes,const fs::path git_path){
    /* std::cout << "Listing:" << std::endl; */
    /* printer(index_tree->directory); */
    for (auto index_node: index_tree->directory){
        if (index_node.type == "blob"){
            bool found = is_in_set(head_leaf_hashes,index_node.hash);
            // This is not in commit set, so print
            if (!found){
                std::cout << "new/modified file: " << index_node.name << std::endl;
            }
            // This node is in the commit set, so should not exist in delete_hashes
            else{
                delete_hashes.erase(index_node.hash);
            }
        }
        else if (index_node.type == "tree"){
            GitTree* subtree = get_tree_from_hash(index_node.hash,git_path);
            print_new_index_nodes_and_calc_delete(subtree,delete_hashes,head_leaf_hashes,git_path);
        }
    }
}

void print_deleted_head_nodes(GitTree* head_tree, const std::set<std::string>& delete_hashes, fs::path rel_path, const fs::path git_path){
    for (auto head_node: head_tree->directory){
        if (head_node.type == "blob"){
            bool deleted = is_in_set(delete_hashes,head_node.hash);
            if (deleted){
                std::cout << "deleted: " << (rel_path / head_node.name) << std::endl;
            }
        }
        else if (head_node.type == "tree"){
            GitTree* subtree = get_tree_from_hash(head_node.hash, git_path);
            print_deleted_head_nodes(subtree,delete_hashes,rel_path / head_node.name, git_path);
        }
        else{
            throw "cpp-git cannot handle this file";
        }
    }
}

void git_status_index_vs_project(const fs::path git_path){
    fs::path project_base_path = repo_find(git_path);

    GitTree* index_tree = get_index_tree(git_path);
    std::set<std::string> index_leaf_hashes;
    get_leaf_hashes_of_tree(index_tree,index_leaf_hashes,git_path);

    std::cout << "---------------Files not yet staged------------" << std::endl;
    print_unstaged_project_files(project_base_path,index_leaf_hashes,git_path,project_base_path);
}

void git_status_commit_index(const fs::path git_path){
    GitTree* index_tree = get_index_tree(git_path);
    GitTree* head_tree = get_head_tree(git_path);
    if (!index_tree || !head_tree){
        std::cout << "Something went wrong" << std::endl;
    }
    std::set<std::string> head_leaf_hashes;
    get_leaf_hashes_of_tree(head_tree,head_leaf_hashes,git_path);

    std::set<std::string> delete_hashes = head_leaf_hashes;
    std::cout << "---------------Files staged for commit:------------" << std::endl;
    print_new_index_nodes_and_calc_delete(index_tree,delete_hashes,head_leaf_hashes,git_path);
    print_deleted_head_nodes(head_tree,delete_hashes,"",git_path);
}

std::string ref_resolve(const fs::path& path, bool return_file_path) {
    std::string data = read_file(path);
    //No content if it's an initial commit
    if (data.size() == 0) return "";
    if (data.rfind("ref: ", 0) == 0)
        return ref_resolve(repo_find(path)/ ".cpp-git" / data.substr(5), return_file_path);
    else if (return_file_path)
        return path;
    else
        return data;
}

std::unordered_map<std::string, std::string> ref_list(const fs::path& base_path) {
    std::unordered_map<std::string, std::string> ret;
    for (auto entry : fs::directory_iterator(base_path)) {
        fs::path cur_path = entry.path();
//        std::cout << cur_path << std::endl;
        if (fs::is_directory(cur_path)) {
            std::unordered_map<std::string, std::string> cur_ret = ref_list(cur_path);
            ret.insert(cur_ret.begin(), cur_ret.end());
        }
        else
            ret[cur_path.string()] = ref_resolve(cur_path);
    }
    return ret;
}

void cmd_show_ref(const std::vector<std::string> &args) {
    fs::path repo_base_path = repo_find(fs::current_path());
    fs::path ref_path = repo_base_path / ".cpp-git" / "refs";
    auto refs = ref_list(ref_path);
    for ( auto it = refs.begin(); it != refs.end(); ++it )
        std::cout << it->second << " refs/" << it->first << std::endl;
    std::cout << std::endl;
}

void cmd_hash_object(const std::vector<std::string> &args) {
    if (args.size() <= 1 || CAT_FILE_SUBCMDS.find(args[0]) == CAT_FILE_SUBCMDS.end() || args[0] == "--help") {
        throw HASH_OBJECT_USAGE;
    }
    git_hash_object(fs::canonical(args[1]), args[0]);
}

void git_hash_object(fs::path file_path, const std::string& fmt) {
    fs::path repo = repo_find(fs::current_path());
    std::string data = read_file(file_path);
    GitObject* obj = create_object(fmt, data, repo);
    write_object(obj, true);
}
