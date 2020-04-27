#include "git_objects.hpp"
#include "helper.hpp"
#include "commands.hpp"

#include <string>
#include <filesystem>
#include <iostream>
#include <vector>
#include <unordered_map>
#include <map>
#include <set>
using namespace std;

void walk_tree_and_replace(fs::path tree_write_path, GitObject* obj) {
    fs::path git_path = repo_find(tree_write_path) / ".cpp-git";

    GitTree* tree_obj = dynamic_cast<GitTree*>(obj);
    cout << "Tree listing: " << endl;
    printer(tree_obj->directory);
    for (auto node : tree_obj->directory) {
        if (node.type == "blob") {
            /* cout << "Write Path: "<< (tree_write_path / node.name) << endl; */
            write_object_to_project_file(tree_write_path / node.name, node.hash);
        } else if (node.type == "tree") {
            /* cout << "Got here" << endl; */
            GitObject* obj = read_object(git_path, node.hash);
            walk_tree_and_replace(tree_write_path / node.name, obj);
        } else {
            cout << "Node should only be tree or blob" << endl;
            throw "Node type should only be tree or blob";
        }
    }
}
/* ********* Traversal Algorithms	********* */

string get_subtree_hash_for_new_file(GitTree* tree_obj, typename fs::path::iterator file_it,
                                const typename fs::path::iterator end_it, const fs::path git_path,
                                const fs::path& file_path) {
    // New Tree Object we are creating
    GitTree new_tree_obj(git_path);

    /* cout << "Currently at: " << *file_it << endl; */
    bool found = false;
    for (auto node : tree_obj->directory) {
        // Case 1: Same branch as file
        if (check_node_name(node,*file_it)) {
            // SubCase 1: file_it is at last element
            bool end = end_of_path(file_it,end_it);
            if (end) {
                // create GitBlob object, write to .git, then add to tree obj
                string blob_hash = read_project_file_and_write_object(git_path, file_path);
                new_tree_obj.add_entry(node.type, node.name, blob_hash);
                found = true;
            }
            // SubCase 2 : still haven't reached file
            else {
                check_if_tree(node);
                GitTree* subtree = get_tree_from_hash(node.hash,git_path);
                auto new_it = file_it;
                string subtree_hash =
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
    cout << "Currently at: " << *file_it << endl;
    if (end_of_path(file_it,end_it) && !found) {
        string blob_hash = read_project_file_and_write_object(git_path, file_path);
        new_tree_obj.add_entry("blob", file_path.filename(), blob_hash);
    }
    // Write Tree and return hash
    cout << "Post Listing: " << endl;
    printer(new_tree_obj.directory);
    return write_object(&new_tree_obj);
}
string get_subtree_hash_for_new_folder(GitTree* tree_obj, typename fs::path::iterator file_it,
                                  typename fs::path::iterator end_it, const fs::path git_path,
                                  const fs::path folder_path) {
    // New Tree Object we are creating
    GitTree new_tree_obj(git_path);

    /* cout << "Pre: currently at " << *file_it << endl; */
    bool found = false;
    for (auto node : tree_obj->directory) {
        // Case 1: Same branch as file
        if (check_node_name(node,*file_it)) {
            // SubCase 1: node points to adding folder
            bool end = end_of_path(file_it,end_it);
            if (end) {
                check_if_tree(node);
                /* cout << "Node name: "  << node.name << endl; */
                string subtree_hash = read_project_folder_and_write_tree(folder_path);
                // add to new_tree
                new_tree_obj.add_entry(node.type, node.name, subtree_hash);
                found = true;
            }
            // SubCase 2 : still haven't reached folder
            else {
                check_if_tree(node);
                GitTree* subtree = get_tree_from_hash(node.hash,git_path);
                auto new_it = file_it;
                string subtree_hash =
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
        /* cout << "Name of mistaken write: " << folder_path.filename() << endl; */
        string subtree_hash = read_project_folder_and_write_tree(folder_path);
        // add to new_tree
        new_tree_obj.add_entry("blob", folder_path.filename(), subtree_hash);
    }
    // Write Tree and return hash
    /* cout << "Post: currently at: " << *file_it << endl; */
    /* printer(new_tree_obj.directory); */
    return write_object(&new_tree_obj);
}
/* ********* 	********* */
int test_function(void) {
    vector<int> test;
    cout << "Hello World" << endl;
    return 0;
}


void cmd_init(const vector<string>& args){
    if ((args.size() >= 1 && args[0] == "--help") || args.size() > 1)
    {
        throw GIT_INIT_USAGE;
    }
    git_init(args[0]);
}


void cmd_add(const vector<string> &args){
    for(auto path: args){
        if (path != "/") path = fs::canonical(path);
        if (fs::is_directory(path)){
            string ERROR = "TODO: add git add folder";
        }
        else{
            git_add_file(path);
        }
    }
}

void cmd_cat_file(const vector<string> &args){
    if (args.size() <= 1 || CAT_FILE_SUBCMDS.find(args[0]) == CAT_FILE_SUBCMDS.end() || args[0] == "--help")
    {
        throw CAT_FILE_USAGE;
    }
    git_cat_file(fs::canonical(args[1]), args[0]);
}

void cmd_checkout(const vector<string>& args){
    if (args.size() != 1)
    {
        throw CHECKOUT_USAGE;
    }
    git_checkout(args[0]);
}

void cmd_commit(const vector<string>& args){
    if (args.size() != 2)
    {
        throw "Currently only support `git commit -m 'your commit message'`";
    }
    git_commit(args[1]);
}

void cmd_reset(const vector<string>& args){
    if (args.size() == 0 || args[0] == "--mixed")
        git_reset(false);
    else
        git_reset(true);
}

void git_cat_file(fs::path obj, const string& fmt){
    fs::path repo = repo_find(fs::current_path());
    GitObject* object = read_object(repo, object_find(repo, obj, fmt));
    cout << object->to_filesystem() << endl;
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

void git_checkout(string hash){
    auto repo_base_path = repo_find(fs::current_path());
    GitObject* obj = read_object(repo_base_path / ".cpp-git", hash);
    string fmt = obj->get_fmt();
    if (fmt == "commit") {
        GitCommit* commit_obj = dynamic_cast<GitCommit*>(obj);
        git_checkout(commit_obj->tree_hash);
    } else {
        throw "Shouldn't reach here";
    }
}

void git_commit(string commit_message){
    fs::path project_base_path = repo_find(fs::current_path());
    fs::path git_path = project_base_path / ".cpp-git";
    
    // Hash of the previous commit (aka HEAD), might be empty if it's the first commit
    string current_branch_name = get_current_branch(git_path);
    string current_commit_hash = get_commit_hash_from_branch(current_branch_name,git_path);

    // Hash of the new tree in index
    string index_tree_hash = get_tree_hash_of_index(git_path);

    GitCommit new_commit_obj = GitCommit(git_path,index_tree_hash,current_commit_hash,commit_message);
    string new_commit_hash = write_object(&new_commit_obj);
    write_file(git_path / current_branch_name, new_commit_hash);
    // clean index for the next round
    write_file(git_path / "index", "");
}

void git_reset(bool hard){
    fs::path worktree = repo_find(fs::current_path());
    fs::path git_path = worktree / ".cpp-git";
    // Get tree_hash from HEAD
    string head_commit_hash = ref_resolve(git_path / "HEAD");
    GitCommit* head_commit = dynamic_cast<GitCommit*>(read_object(git_path, head_commit_hash));
    string head_commit_tree_hash = head_commit->tree_hash;
    write_file(git_path / "index", head_commit_tree_hash);

    if (hard){
        GitTree* tree = dynamic_cast<GitTree*>(read_object(git_path, head_commit_tree_hash));
        walk_tree_and_replace(worktree, tree);
    }
}

// NOTE: can add new files, but they must be in existing folders already
string git_add_file(const fs::path& file_path) {
    fs::path project_base_path = repo_find(file_path);
    fs::path git_path = project_base_path / ".cpp-git";
    string new_tree_hash;

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
            string blob_hash = read_project_file_and_write_object(git_path,file_path);
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

    cout << "Should write to index now" << endl;
    write_file(git_path / "index", new_tree_hash);
    return new_tree_hash;
}


string git_add_folder(const fs::path folder_path) {
    fs::path project_base_path = repo_find(folder_path);
    fs::path git_path = project_base_path / ".cpp-git";
    string new_tree_hash;

    // Run file_it
    auto base_it = project_base_path.begin();
    auto file_it = folder_path.begin();
    while (base_it != project_base_path.end()) {
        base_it++;
        file_it++;
    }

    // Common Case of just adding from project root
    // in which case, we want index = working
    if (folder_path == project_base_path){
        new_tree_hash = read_project_folder_and_write_tree(folder_path);
    }
    else{
        GitTree* index_tree = get_index_tree(git_path);
        if (!index_tree){
            GitTree* head_tree = get_head_tree(git_path);
            if (!head_tree){
                    throw "git add error";
            }
            else{
                // SubCase 2: then base off head tree instead
                /* cout << "DEBUG: " << "no index but yes head" << endl; */
                new_tree_hash = get_subtree_hash_for_new_folder(head_tree,file_it,folder_path.end(),git_path,folder_path);
            }
        }
        else{
            new_tree_hash =
                get_subtree_hash_for_new_folder(index_tree, file_it, folder_path.end(), git_path, folder_path);
        }
    }

    cout << "Should write to index now" << endl;
    write_file(git_path / "index", new_tree_hash);
    return new_tree_hash;
}

void get_project_file_hashes(fs::path directory,vector<string>& project_leaf_hashes,unordered_map<string,string>& path_to_hash_dict, const fs::path git_path){
    for (auto entry: fs::directory_iterator(directory)){
        fs::path entry_path = entry.path();
        if (fs::is_regular_file(entry_path)){
            string file_hash = read_project_file_and_write_object(git_path,entry_path);
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



/* void print_new_hashes(fs::path directory, const set<string>& diff_hashes,const unordered_map<string,string>& path_to_hash_dict,const fs::path project_base_path){ */
/*     cout << "---------------Unstaged changes------------" << endl; */
/*     for (auto entry: fs::directory_iterator(directory)){ */
/*         fs::path entry_path = entry.path(); */
/*         auto cached_hash = path_to_hash_dict.at(entry_path.string()); */
/*         if (fs::is_regular_file(entry_path)){ */
/*             bool found = isInSet(diff_hashes,cached_hash); */
/*             if (found){ */
/*                 cout << "new/modified file: " << path_relative_to_project(project_base_path,entry_path) << endl; */
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

set<string> set_difference_of_leaf_hashes(vector<string>& leaf_hashes_1, vector<string>& leaf_hashes_2){
    sort(leaf_hashes_1.begin(),leaf_hashes_1.end());
    sort(leaf_hashes_2.begin(),leaf_hashes_2.end());

    vector<string> diff_hashes;
    set_difference(leaf_hashes_1.begin(),leaf_hashes_1.end(),leaf_hashes_2.begin(),leaf_hashes_2.end(),back_inserter(diff_hashes));
    return set<string>(diff_hashes.begin(),diff_hashes.end());
}

/* ********* 	********* */
void get_leaf_hashes_of_tree(GitTree* tree_obj,set<string>& index_leaf_hashes, const fs::path git_path){
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
void get_leaf_hashes_of_tree(GitTree* tree_obj,map<string,string>& leaf_hashes, fs::path current_path,const fs::path git_path){
    for (auto node: tree_obj->directory){
        string path_name = string(current_path / node.name);
        if (node.type=="blob"){
            leaf_hashes.insert({node.hash,path_name});
        }
        else if (node.type == "tree"){
            GitTree* subtree = get_tree_from_hash(node.hash,git_path);
            get_leaf_hashes_of_tree(subtree,leaf_hashes,path_name, git_path);
        }
        else{
            throw "cpp-git cannot handle this file";
        }
    }
}

void print_unstaged_project_files(fs::path directory, const set<string>& index_leaf_hashes, const fs::path git_path, const fs::path project_base_path){
    for(auto project_file: fs::directory_iterator(directory)){
        fs::path project_file_path = project_file.path();
        /* cout << "file path: " << project_file_path << endl; */
        if(is_git_repo(project_file_path)){
            continue;
        }

        if (fs::is_regular_file(project_file_path)){
            string file_hash = read_project_file_and_write_object(git_path,project_file_path);
            if (!is_in_set(index_leaf_hashes,file_hash)){
                cout <<  path_relative_to_project(project_base_path,project_file_path) << endl;
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

void print_new_index_nodes_and_calc_delete(GitTree* tree,set<string>& delete_hashes, const set<string>& head_leaf_hashes,const fs::path git_path){
    /* cout << "Listing:" << endl; */
    /* printer(tree->directory); */
    for (auto index_node: tree->directory){
        if (index_node.type == "blob"){
            bool found = is_in_set(head_leaf_hashes,index_node.hash);
            // This is not in commit set, so print
            if (!found){
                cout << "new/modified file: " << index_node.name << endl;
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

void walk_index_and_calc_set_differences(GitTree* tree, map<string,string>& commit_diff_hashes, map<string,string>& index_diff_hashes, fs::path current_path, const map<string,string>& head_leaf_hashes, const fs::path git_path){
    for (auto index_node: tree->directory){
        fs::path new_path = current_path / index_node.name;
        if (index_node.type == "blob"){
            bool index_hash_not_common = is_in_set(head_leaf_hashes,index_node.hash);
            // This is not in commit set, so print
            if (!index_hash_not_common){
                index_diff_hashes.insert({index_node.hash,new_path});
            }
            // This node is in the commit set, so should not exist in delete_hashes
            else{
                commit_diff_hashes.erase(index_node.hash);
            }
        }
        else if (index_node.type == "tree"){
            GitTree* subtree = get_tree_from_hash(index_node.hash,git_path);
            walk_index_and_calc_set_differences(subtree,commit_diff_hashes,index_diff_hashes,new_path,head_leaf_hashes,git_path);
        }
    }

}

void print_deleted_head_nodes(GitTree* head_tree, const set<string>& delete_hashes, fs::path rel_path, const fs::path git_path){
    for (auto head_node: head_tree->directory){
        if (head_node.type == "blob"){
            bool deleted = is_in_set(delete_hashes,head_node.hash);
            if (deleted){
                cout << "deleted: " << (rel_path / head_node.name) << endl;
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

vector<string> convert_map_to_sorted_values(map<string,string> my_map){
    vector<string> output;
    output.reserve(my_map.size());
    for (auto& pair:my_map){
        output.push_back(pair.second);
    }
    std::sort(output.begin(),output.end());
    return output;
}

void split_into_deleted_modified_new(map<string,string>& commit_diff_hashes, map<string,string>& index_diff_hashes,const fs::path project_base_path){
    vector<string> commit_paths = convert_map_to_sorted_values(commit_diff_hashes);
    vector<string> index_paths = convert_map_to_sorted_values(index_diff_hashes);

    vector<string> modified_files;
    std::set_intersection(commit_paths.begin(),commit_paths.end(),index_paths.begin(),index_paths.end(),std::back_inserter(modified_files));
    for(auto path:modified_files){
        cout << "modified: " << path_relative_to_project(project_base_path,path) << endl;
    }

    vector<string> deleted_files;
    std::set_difference(commit_paths.begin(),commit_paths.end(),index_paths.begin(),index_paths.end(),std::back_inserter(deleted_files));
    for(auto path:deleted_files){
        cout << "deleted: " << path_relative_to_project(project_base_path,path) << endl;
    }

    vector<string> new_files;
    std::set_difference(index_paths.begin(),index_paths.end(),commit_paths.begin(),commit_paths.end(),std::back_inserter(new_files));
    for(auto path:new_files){
        cout << "new: " << path_relative_to_project(project_base_path,path) << endl;
    }

}

void git_status_index_vs_project(const fs::path git_path){
    fs::path project_base_path = repo_find(git_path);
    GitTree* index_tree = get_index_tree(git_path);
    set<string> index_leaf_hashes;

    cout << "---------------Files not yet staged------------" << endl;
    if (!index_tree){
        // pass in empty index leaf hash
        print_unstaged_project_files(project_base_path,index_leaf_hashes,git_path,project_base_path);
    }
    else{
        get_leaf_hashes_of_tree(index_tree,index_leaf_hashes,git_path);
        print_unstaged_project_files(project_base_path,index_leaf_hashes,git_path,project_base_path);
    }
}

void git_status_commit_index(const fs::path git_path){
    cout << "---------------Files staged for commit:------------" << endl;
    // EC: no index
    // EC : no head
    fs::path project_base_path = repo_find(git_path);
    GitTree* index_tree = get_index_tree(git_path);
    GitTree* head_tree = get_head_tree(git_path);

    if (index_tree){
        if (!head_tree){
            set<string> delete_hashes;
            set<string> set_head_hashes;
            // Pass empty set_head_hashes so everything is new
            print_new_index_nodes_and_calc_delete(index_tree,delete_hashes,set_head_hashes,git_path);
        }
        else{
            map<string,string> head_leaf_hashes;
            map<string,string> commit_diff_hashes;
            map<string,string> index_diff_hashes;
            // COMMON CASE, both index and head trees exist
            // TODO: overload
            get_leaf_hashes_of_tree(head_tree,head_leaf_hashes,project_base_path,git_path);
            commit_diff_hashes = head_leaf_hashes;
            walk_index_and_calc_set_differences(index_tree,commit_diff_hashes,index_diff_hashes,project_base_path,head_leaf_hashes,git_path);

            split_into_deleted_modified_new(commit_diff_hashes,index_diff_hashes,project_base_path);
        }
    }
    // Otherwise there is nothing staged
}

string ref_resolve(const fs::path& path, bool return_file_path) {
    string data = read_file(path);
    //No content if it's an initial commit
    if (data.size() == 0) return "";
    if (data.rfind("ref: ", 0) == 0)
        return ref_resolve(repo_find(path)/ ".cpp-git" / data.substr(5), return_file_path);
    else if (return_file_path)
        return path;
    else
        return data;
}

unordered_map<string, string> ref_list(const fs::path& base_path) {
    unordered_map<string, string> ret;
    for (auto entry : fs::directory_iterator(base_path)) {
        fs::path cur_path = entry.path();
//        cout << cur_path << endl;
        if (fs::is_directory(cur_path)) {
            unordered_map<string, string> cur_ret = ref_list(cur_path);
            ret.insert(cur_ret.begin(), cur_ret.end());
        }
        else
            ret[cur_path.string()] = ref_resolve(cur_path);
    }
    return ret;
}

void cmd_show_ref(const vector<string> &args) {
    fs::path repo_base_path = repo_find(fs::current_path());
    fs::path ref_path = repo_base_path / ".cpp-git" / "refs";
    auto refs = ref_list(ref_path);
    for ( auto it = refs.begin(); it != refs.end(); ++it )
        cout << it->second << " refs/" << it->first << endl;
    cout << endl;
}

void cmd_hash_object(const vector<string> &args) {
    if (args.size() <= 1 || CAT_FILE_SUBCMDS.find(args[0]) == CAT_FILE_SUBCMDS.end() || args[0] == "--help") {
        throw HASH_OBJECT_USAGE;
    }
    git_hash_object(fs::canonical(args[1]), args[0]);
}

void git_hash_object(fs::path file_path, const string& fmt) {
    fs::path repo = repo_find(fs::current_path());
    string data = read_file(file_path);
    GitObject* obj = create_object(fmt, data, repo);
    write_object(obj, true);
}
