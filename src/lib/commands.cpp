#include "commands.hpp"

#include <climits>
#include <filesystem>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>
#include <stdexcept>

#include "git_objects.hpp"
#include "helper.hpp"
using namespace std;

static void walk_tree_and_replace(fs::path tree_write_path, GitTree tree_obj) {
    fs::path git_path = repo_find(tree_write_path) / ".cpp-git";

    /* cout << "Tree listing: " << endl; */
    /* printer(tree_obj->directory); */
    for (const auto& node : tree_obj.directory) {
        if (node.type == "blob") {
            /* cout << "Write Path: "<< (tree_write_path / node.name) << endl; */
            write_object_to_project_file(tree_write_path / node.name, node.hash);
        } else if (node.type == "tree") {
            /* cout << "Got here" << endl; */
            GitTree subtree;
            read_into_object(subtree, git_path, node.hash);
            walk_tree_and_replace(tree_write_path / node.name, subtree);
        } else {
            cout << "Node should only be tree or blob" << endl;
            throw std::runtime_error("Node type should only be tree or blob");
        }
    }
}
/* ********* Traversal Algorithms	********* */

static string get_subtree_hash_for_new_file(GitTree* tree_obj, typename fs::path::iterator file_it,
                                            const fs::path& file_path, const Context context) {
    // New Tree Object we are creating
    GitTree new_tree_obj(context.git_path);

    /* cout << "Currently at: " << *file_it << endl; */
    bool found = false;
    for (const auto& node : tree_obj->directory) {
        // Case 1: Same branch as file
        if (check_node_name(node, *file_it)) {
            // SubCase 1: file_it is at last element
            bool end = end_of_path(file_it, context.end_it);
            if (end) {
                // create GitBlob object, write to .git, then add to tree obj
                string blob_hash = read_project_file_and_write_object(context.git_path, file_path);
                new_tree_obj.add_entry(node.type, node.name, blob_hash);
                found = true;
            }
            // SubCase 2 : still haven't reached file
            else {
                check_if_tree(node);
                GitTree subtree;
                read_into_object(subtree, context.git_path, node.hash);
                auto new_it = file_it;
                string subtree_hash =
                    get_subtree_hash_for_new_file(&subtree, ++new_it, file_path, context);
                new_tree_obj.add_entry("tree", node.name, subtree_hash);
            }
        }
        // Case 2: Not what we want to add to index
        else {
            new_tree_obj.add_entry(node.type, node.name, node.hash);
        }
    }
    // EC: if at end and we haven't found the file in the old tree directory
    /* cout << "Currently at: " << *file_it << endl; */
    if (end_of_path(file_it, context.end_it) && !found) {
        string blob_hash = read_project_file_and_write_object(context.git_path, file_path);
        new_tree_obj.add_entry("blob", file_path.filename(), blob_hash);
    }
    // Write Tree and return hash
    /* cout << "Post Listing: " << endl; */
    /* printer(new_tree_obj.directory); */
    return write_object(&new_tree_obj);
}
static string get_subtree_hash_for_new_folder(GitTree* tree_obj,
                                              typename fs::path::iterator file_it,
                                              const fs::path folder_path, Context context) {
    // New Tree Object we are creating
    GitTree new_tree_obj(context.git_path);

    /* cout << "Pre: currently at " << *file_it << endl; */
    bool found = false;
    for (const auto & node : tree_obj->directory) {
        // Case 1: Same branch as file
        if (check_node_name(node, *file_it)) {
            // SubCase 1: node points to adding folder
            bool end = end_of_path(file_it, context.end_it);
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
                GitTree subtree;
                read_into_object(subtree, context.git_path, node.hash);
                auto new_it = file_it;
                string subtree_hash =
                    get_subtree_hash_for_new_folder(&subtree, ++new_it, folder_path, context);
                new_tree_obj.add_entry("tree", node.name, subtree_hash);
            }
        }
        // Case 2: Not what we want to add to index
        else {
            new_tree_obj.add_entry(node.type, node.name, node.hash);
        }
    }

    // TODO: extract this out of the recursion, as technically I shouldn't create a local found
    // variable everytime
    // EC: if at end and we haven't found the file in the old tree directory
    if (end_of_path(file_it, context.end_it) && !found) {
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

void git_checkout_file(fs::path file_path) {
    fs::path project_base_path = repo_find(file_path);
    fs::path git_path = project_base_path / ".cpp-git";

    string full_branch_name = get_current_branch_full(git_path);
    string commit_hash = get_commit_hash_from_branch(full_branch_name, git_path);
    Option<GitTree> option_head_tree = get_head_tree(git_path);
    if (!option_head_tree.exists) {
        throw std::runtime_error("Git checkout error.Make a commit first");
    }
    // Run file_it
    auto base_it = project_base_path.begin();
    auto file_it = file_path.begin();
    while (base_it != project_base_path.end()) {
        base_it++;
        file_it++;
    }

    string file_hash =
        find_hash_in_tree(&option_head_tree.content, file_it, file_path.end(), git_path);
    if (file_hash != "") {
        /* cout << "File has not previously been checked in"; */
        write_object_to_project_file(file_path, file_hash);
    } else {
        throw std::runtime_error("error. File has not previously been checked in");
    }
}
bool is_actually_a_hash(string branch_name, fs::path git_path) {
    if (branch_name.length() < 4) {
        return false;
    }
    // since git stores each object with the first 2 letters of the hash as the folder name,
    // and the rest as the file name
    return fs::exists(git_path / "objects" / branch_name.substr(0,2) / branch_name.substr(2));
}
void git_checkout_branch(string branch_name) {
    fs::path project_base_path = repo_find(fs::current_path());
    fs::path git_path = project_base_path / ".cpp-git";

    string commit_hash;
    string full_branch_name = "refs/heads/" + branch_name;
    Option<GitTree> option_index_tree = get_index_tree(git_path);
    if (option_index_tree.exists) {
        throw std::runtime_error("error. Please commit changes before changing branches");
    }

    // if we want to check out via a hash rather than a name
    if (is_actually_a_hash(branch_name, git_path)) {
        commit_hash = branch_name;
    } else {
        // will throw if not a valid branch
        commit_hash = get_commit_hash_from_branch(full_branch_name, git_path);
    }
    GitCommit commit = get_commit_from_hash(commit_hash, git_path);
    GitTree commit_tree;
    read_into_object(commit_tree, git_path, commit.tree_hash);
    walk_tree_and_replace(project_base_path, commit_tree);
    // now update HEAD as we have switched over
    write_file(git_path / "HEAD", "ref: " + full_branch_name);
}

void git_branch_new(string branch_name) {
    fs::path project_base_path = repo_find(fs::current_path());
    fs::path git_path = project_base_path / ".cpp-git";

    fs::path full_branch_path = git_path / get_full_branch_name(branch_name);
    if (fs::exists(full_branch_path)) {
        throw std::runtime_error("error. branch already exists");
    }

    // make new branch point to current branch's commit
    string current_full_branch_name = get_current_branch_full(git_path);
    string current_commit_hash = get_commit_hash_from_branch(current_full_branch_name, git_path);
    write_file(full_branch_path, current_commit_hash);
}

void git_branch_delete(string branch_name) {
    fs::path project_base_path = repo_find(fs::current_path());
    fs::path git_path = project_base_path / ".cpp-git";

    fs::path full_branch_path = git_path / get_full_branch_name(branch_name);
    if (!fs::exists(full_branch_path)) {
        throw std::runtime_error("error. branch does not exist");
    }

    fs::remove(full_branch_path);
}

void git_branch_list() {
    fs::path project_base_path = repo_find(fs::current_path());
    fs::path git_path = project_base_path / ".cpp-git";
    fs::path full_branch_name = get_current_branch_full(git_path);

    std::cout << "Branches:" << std::endl;
    for (const auto  & entry : fs::directory_iterator(git_path / "refs/heads")) {
        /* std::cout << "Full branch name's filename" << full_branch_name.filename() << std::endl;
         */
        /* std::cout << "Entry's filename" << entry.path().filename() << std::endl; */
        if (entry.path().filename() == full_branch_name.filename()) {
            std::cout << "-> " << entry.path().filename() << std::endl;
        } else {
            std::cout << entry.path().filename() << std::endl;
        }
    }
}
/* ********* 	********* */
int test_function(void) {
    vector<int> test;
    cout << "Hello World" << endl;
    return 0;
}

void git_cat_file(fs::path obj, const string& fmt) {
    fs::path repo = repo_find(fs::current_path());
    GitObject* object = read_object(repo, object_find(repo, obj, fmt));
    cout << object->to_filesystem() << endl;
}

void git_init(fs::path project_base_path) {
    // check that .git doesn't exist or is empty directory
    fs::path git_path = project_base_path / ".cpp-git";
    if (fs::exists(git_path)) {
        throw std::runtime_error("init error: Not an empty path");
    }
    // create object dir
    fs::create_directories(git_path / "objects");
    // create HEAD file with "ref: refs/heads/master"
    write_file(git_path / "HEAD", "ref: refs/heads/master");
    write_file(git_path / "index", "");
    write_file(git_path / "refs" / "heads" / "master", "");

    // create branches dir
    fs::create_directories(git_path / "branches");
    // create refs dir with tags+heads subdirectory
    fs::create_directories(git_path / "refs" / "tags");
}

void git_commit(string commit_message) {
    fs::path project_base_path = repo_find(fs::current_path());
    fs::path git_path = project_base_path / ".cpp-git";

    string current_full_branch_name = get_current_branch_full(git_path);
    // Hash of the previous commit (aka HEAD), might be empty if it's the first commit
    string current_commit_hash = get_commit_hash_from_branch(current_full_branch_name, git_path);

    // Hash of the new tree in index
    string index_tree_hash = get_tree_hash_of_index(git_path);
    if (index_tree_hash.length() == 0) {
        std::cout << "Error. Nothing to commit" << std::endl;
    } else {
        GitCommit new_commit_obj =
            GitCommit(git_path, std::move(index_tree_hash), std::move(current_commit_hash), std::move(commit_message));
        string new_commit_hash = write_object(&new_commit_obj);
        // move ref to new commit
        write_file(git_path / current_full_branch_name, new_commit_hash);
        // clean index for the next round
        write_file(git_path / "index", "");
    }
}

#if 0
static string get_prev_commit_hash_from_branch(string current_full_branch_name, fs::path git_path){
    string current_commit_hash = get_commit_hash_from_branch(current_full_branch_name,git_path);
    if (current_commit_hash==""){
        throw std::runtime_error("git amend error. Make a commit first");
    }
    GitCommit current_commit = get_commit_from_hash(current_commit_hash,git_path);
    return current_commit.parent_hash;
}
#endif

void git_amend(string commit_message) {
    fs::path project_base_path = repo_find(fs::current_path());
    fs::path git_path = project_base_path / ".cpp-git";

    // 1. Getting Acess to Commit Objects
    string current_full_branch_name = get_current_branch_full(git_path);
    string current_commit_hash = get_commit_hash_from_branch(current_full_branch_name, git_path);
    if (current_commit_hash == "") {
        throw std::runtime_error("git amend error. Make a commit first");
    }
    GitCommit current_commit = get_commit_from_hash(current_commit_hash, git_path);
    string prev_commit_hash = current_commit.parent_hash;

    // 2. Actually creating the commit
    string tree_hash = get_tree_hash_of_index(git_path);
    if (tree_hash.length() == 0) {
        // then grab tree from current commit
        tree_hash = current_commit.tree_hash;
    } else {
        GitCommit new_commit_obj = GitCommit(git_path, std::move(tree_hash), std::move(prev_commit_hash), std::move(commit_message));
        string new_commit_hash = write_object(&new_commit_obj);
        // move ref to new commit
        write_file(git_path / current_full_branch_name, new_commit_hash);
        // clean index for the next round
        write_file(git_path / "index", "");
    }
}

class UnStager {
   private:
    bool* found;
    fs::path file_path;
    bool replace;

   public:
    UnStager(bool* found, fs::path file_path, bool replace) {
        this->found = found;
        this->file_path = file_path;
        this->replace = replace;
    }
    string unstage_file(GitTree* tree_obj, typename fs::path::iterator file_it, Context context);
};
string UnStager::unstage_file(GitTree* tree_obj, typename fs::path::iterator file_it,
                              Context context) {
    GitTree new_tree_obj(context.git_path);

#if 1
    for (const auto & node : tree_obj->directory) {
        // Case 1: Same branch as file
        if (check_node_name(node, *file_it)) {
            bool end = end_of_path(
                file_it, context.end_it);  // NOTE: cannot just call file_path.end() for some reason
            if (end) {
                *found = true;
                if (replace) {
                    // actually no need since caller(git_reset_file) will already check
                    // check_if_tree_exists(option_head_tree);
                    string old_file_hash =
                        find_hash_in_tree(&context.head_tree, context.start_file_it, context.end_it,
                                          context.git_path);
                    new_tree_obj.add_entry("blob", node.name, old_file_hash);
                    /* GitBlob file; */
                    /* read_into_object(file,git_path,old_file_hash); */
                    /* std::cout << file.data << std::endl; */
                }
                // else do nothing/don't add to tree listing
            } else {
                check_if_tree(node);
                GitTree subtree;
                read_into_object(subtree, context.git_path, node.hash);
                string subtree_hash = this->unstage_file(&subtree, ++file_it, context);
                new_tree_obj.add_entry("tree", node.name, subtree_hash);
            }
        } else {
            new_tree_obj.add_entry(node.type, node.name, node.hash);
        }
    }
#endif
    return write_object(&new_tree_obj);
}

string git_reset_file(fs::path file_path, bool hard) {
    fs::path project_base_path = repo_find(fs::current_path());
    fs::path git_path = project_base_path / ".cpp-git";
    string new_index_tree_hash;

    // Run file_it
    auto base_it = project_base_path.begin();
    /* typename fs::path::iterator file_it = file_path.begin(); */
    auto file_it = file_path.begin();
    while (base_it != project_base_path.end()) {
        base_it++;
        file_it++;
    }
    auto start_file_it = file_it;
    auto end_it = file_path.end();

    bool found = false;
    Option<GitTree> option_index_tree = get_index_tree(git_path);
    if (option_index_tree.exists) {
        GitTree* index_tree = &option_index_tree.content;
        Option<GitTree> option_head_tree = get_head_tree(git_path);
        if (option_head_tree.exists) {
            // SubCase 1: replace with head's version/hash at file_path's location
            Context context{option_head_tree.content, start_file_it, end_it, git_path,
                            project_base_path};
            UnStager unstager{&found, file_path, true};
            new_index_tree_hash = unstager.unstage_file(index_tree, file_it, context);
        } else {
            // SubCase 2: just ignore during traversal
            GitTree dummy_tree;
            Context context{dummy_tree, start_file_it, end_it, git_path, project_base_path};
            UnStager unstager{&found, file_path, false};
            new_index_tree_hash = unstager.unstage_file(index_tree, file_it, context);
        }
    } else {
        throw std::runtime_error("nothing to reset in index");
    }

    if (!found) {
        throw std::runtime_error("this file does not exist in the index");
    }

    if (hard) {
        git_checkout_file(file_path);
    }
    write_file(git_path / "index", new_index_tree_hash);
    return new_index_tree_hash;
}

void git_reset_project(bool hard) {
    fs::path project_base_path = repo_find(fs::current_path());
    fs::path git_path = project_base_path / ".cpp-git";
    // Get tree_hash from HEAD
    string full_branch_name = get_current_branch_full(git_path);
    string head_commit_hash = get_commit_hash_from_branch(full_branch_name, git_path);
    GitCommit head_commit;
    read_into_object(head_commit, git_path, head_commit_hash);
    string head_commit_tree_hash = head_commit.tree_hash;
    write_file(git_path / "index", "");

    if (hard) {
        GitTree tree;
        read_into_object(tree, git_path, head_commit_tree_hash);
        walk_tree_and_replace(project_base_path, tree);
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
    Option<GitTree> option_index_tree = get_index_tree(git_path);
    Context context;
    context.end_it = file_path.end();
    context.git_path = git_path;
    if (!option_index_tree.exists) {
        // SubCase 1: if even head is empty, just add from project folder instead of traversing git
        // trees
        Option<GitTree> option_head_tree = get_head_tree(git_path);
        if (!option_head_tree.exists) {
            // TODO: check that folder path and project base path are the same
            string blob_hash = read_project_file_and_write_object(git_path, file_path);
            GitTree tree_obj(git_path);
            tree_obj.add_entry("blob", file_path.filename(), blob_hash);
            new_tree_hash = write_object(&tree_obj);
            write_file(git_path / "index", new_tree_hash);
        } else {
            // SubCase 2: then base off head tree instead
            new_tree_hash = get_subtree_hash_for_new_file(&option_head_tree.content, file_it,
                                                          file_path, context);
        }
    } else {
        new_tree_hash =
            get_subtree_hash_for_new_file(&option_index_tree.content, file_it, file_path, context);
    }
    /* cout << "Should write to index now" << endl; */
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
    if (folder_path == project_base_path) {
        new_tree_hash = read_project_folder_and_write_tree(folder_path);
    } else {
        Option<GitTree> option_index_tree = get_index_tree(git_path);
        if (!option_index_tree.exists) {
            Option<GitTree> option_head_tree = get_head_tree(git_path);
            if (!option_head_tree.exists) {
                throw std::runtime_error("git add error");
            } else {
                // SubCase 2: then base off head tree instead
                /* cout << "DEBUG: " << "no index but yes head" << endl; */
                Context context;
                context.end_it = folder_path.end();
                context.git_path = git_path;
                new_tree_hash = get_subtree_hash_for_new_folder(&option_head_tree.content, file_it,
                                                                folder_path, context);
            }
        } else {
            Context context;
            context.end_it = folder_path.end();
            context.git_path = git_path;
            new_tree_hash = get_subtree_hash_for_new_folder(&option_index_tree.content, file_it,
                                                            folder_path, context);
        }
    }

    /* cout << "Should write to index now" << endl; */
    write_file(git_path / "index", new_tree_hash);
    return new_tree_hash;
}

/* void get_project_file_hashes(fs::path directory,vector<string>&
 * project_leaf_hashes,unordered_map<string,string>& path_to_hash_dict, const fs::path git_path){ */
/*     for (auto entry: fs::directory_iterator(directory)){ */
/*         fs::path entry_path = entry.path(); */
/*         if (fs::is_regular_file(entry_path)){ */
/*             string file_hash = read_project_file_and_write_object(git_path,entry_path); */
/*             project_leaf_hashes.push_back(file_hash); */
/*             path_to_hash_dict.insert({entry_path.string(),file_hash}); */
/*         } */
/*         else if (fs::is_directory(entry_path)){ */
/*             get_project_file_hashes(entry_path,project_leaf_hashes,path_to_hash_dict,git_path);
 */
/*         } */
/*         else{ */
/*             throw std::runtime_error("cpp-git cannot handle this file"); */
/*         } */
/*     } */
/* } */

/* void print_new_hashes(fs::path directory, const set<string>& diff_hashes,const
 * unordered_map<string,string>& path_to_hash_dict,const fs::path project_base_path){ */
/*     cout << "---------------Unstaged changes------------" << endl; */
/*     for (auto entry: fs::directory_iterator(directory)){ */
/*         fs::path entry_path = entry.path(); */
/*         auto cached_hash = path_to_hash_dict.at(entry_path.string()); */
/*         if (fs::is_regular_file(entry_path)){ */
/*             bool found = isInSet(diff_hashes,cached_hash); */
/*             if (found){ */
/*                 cout << "new/modified file: " <<
 * path_relative_to_project(project_base_path,entry_path) << endl; */
/*             } */
/*         } */
/*         else if (fs::is_directory(entry_path)){ */
/*             print_new_hashes(entry_path,diff_hashes,path_to_hash_dict,project_base_path); */
/*         } */
/*         else{ */
/*             throw std::runtime_error("cpp-git cannot handle this file"); */
/*         } */
/*     } */
/* } */

/* set<string> set_difference_of_leaf_hashes(vector<string>& leaf_hashes_1, vector<string>&
 * leaf_hashes_2){ */
/*     sort(leaf_hashes_1.begin(),leaf_hashes_1.end()); */
/*     sort(leaf_hashes_2.begin(),leaf_hashes_2.end()); */

/*     vector<string> diff_hashes; */
/*     set_difference(leaf_hashes_1.begin(),leaf_hashes_1.end(),leaf_hashes_2.begin(),leaf_hashes_2.end(),back_inserter(diff_hashes));
 */
/*     return set<string>(diff_hashes.begin(),diff_hashes.end()); */
/* } */

/* ********* 	********* */
static void get_leaf_hashes_of_tree(GitTree* tree_obj, set<string>& index_leaf_hashes,
                                    const fs::path git_path) {
    for (const auto & node : tree_obj->directory) {
        if (node.type == "blob") {
            index_leaf_hashes.insert(node.hash);
        } else if (node.type == "tree") {
            GitTree subtree;
            read_into_object(subtree, git_path, node.hash);
            get_leaf_hashes_of_tree(&subtree, index_leaf_hashes, git_path);
        } else {
            throw std::runtime_error("cpp-git cannot handle this file");
        }
    }
}
static void get_leaf_hashes_of_tree(GitTree* tree_obj, map<string, string>& leaf_hashes,
                                    fs::path current_path, const fs::path git_path) {
    for (const auto & node : tree_obj->directory) {
        string path_name = string(current_path / node.name);
        if (node.type == "blob") {
            leaf_hashes.insert({node.hash, path_name});
        } else if (node.type == "tree") {
            GitTree subtree;
            read_into_object(subtree, git_path, node.hash);
            get_leaf_hashes_of_tree(&subtree, leaf_hashes, path_name, git_path);
        } else {
            throw std::runtime_error("cpp-git cannot handle this file");
        }
    }
}

static void print_unstaged_project_files(fs::path directory, const set<string>& index_leaf_hashes,
                                         const fs::path git_path,
                                         const fs::path project_base_path) {
    for (const auto & project_file : fs::directory_iterator(directory)) {
        const fs::path & project_file_path = project_file.path();
        /* cout << "file path: " << project_file_path << endl; */
        if (is_git_repo(project_file_path)) {
            continue;
        }

        if (fs::is_regular_file(project_file_path)) {
            string file_hash = read_project_file_and_write_object(git_path, project_file_path);
            if (!is_in_set(index_leaf_hashes, file_hash)) {
                cout << path_relative_to_project(project_base_path, project_file_path) << endl;
            }
        } else if (fs::is_directory(project_file_path)) {
            print_unstaged_project_files(project_file_path, index_leaf_hashes, git_path,
                                         project_base_path);
        } else {
            throw std::runtime_error("cpp-git cannot handle this file");
        }
    }
}

static void print_new_index_nodes_and_calc_delete(GitTree* tree, set<string>& delete_hashes,
                                                  const set<string>& head_leaf_hashes,
                                                  const fs::path git_path) {
    /* cout << "Listing:" << endl; */
    /* printer(tree->directory); */
    for (const auto & index_node : tree->directory) {
        if (index_node.type == "blob") {
            bool found = is_in_set(head_leaf_hashes, index_node.hash);
            // This is not in commit set, so print
            if (!found) {
                cout << "new/modified file: " << index_node.name << endl;
            }
            // This node is in the commit set, so should not exist in delete_hashes
            else {
                delete_hashes.erase(index_node.hash);
            }
        } else if (index_node.type == "tree") {
            GitTree subtree;
            read_into_object(subtree, git_path, index_node.hash);
            print_new_index_nodes_and_calc_delete(&subtree, delete_hashes, head_leaf_hashes,
                                                  git_path);
        }
    }
}

static void walk_index_and_calc_set_differences(GitTree* tree, fs::path current_path,
                                                map<string, string>& commit_diff_hashes,
                                                map<string, string>& index_diff_hashes,
                                                const map<string, string>& head_leaf_hashes,
                                                const fs::path git_path) {
    /* std::cout << "Index Tree Listing:" << std::endl; */
    /* printer(tree->directory); */
    for (const auto & index_node : tree->directory) {
        fs::path new_path = current_path / index_node.name;
        if (index_node.type == "blob") {
            bool index_hash_not_common = is_in_set(head_leaf_hashes, index_node.hash);
            // This is not in commit set, so print
            if (!index_hash_not_common) {
                index_diff_hashes.insert({index_node.hash, new_path});
            }
            // This node is in the commit set, so should not exist in delete_hashes
            else {
                commit_diff_hashes.erase(index_node.hash);
            }
        } else if (index_node.type == "tree") {
            GitTree subtree;
            read_into_object(subtree, git_path, index_node.hash);
            walk_index_and_calc_set_differences(&subtree, new_path, commit_diff_hashes,
                                                index_diff_hashes, head_leaf_hashes, git_path);
        }
    }
}

//static void print_deleted_head_nodes(GitTree* head_tree, const set<string>& delete_hashes,
                                     //fs::path rel_path, const fs::path git_path) {
    //for (auto head_node : head_tree->directory) {
        //if (head_node.type == "blob") {
            //bool deleted = is_in_set(delete_hashes, head_node.hash);
            //if (deleted) {
                //cout << "deleted: " << (rel_path / head_node.name) << endl;
            //}
        //} else if (head_node.type == "tree") {
            //GitTree subtree;
            //read_into_object(subtree, git_path, head_node.hash);
            //print_deleted_head_nodes(&subtree, delete_hashes, rel_path / head_node.name, git_path);
        //} else {
            //throw std::runtime_error("cpp-git cannot handle this file");
        //}
    //}
//}

static vector<string> convert_map_to_sorted_values(map<string, string> my_map) {
    vector<string> output;
    output.reserve(my_map.size());
    for (const auto& pair : my_map) {
        output.push_back(pair.second);
    }
    std::sort(output.begin(), output.end());
    return output;
}

static void split_into_deleted_modified_new(map<string, string>& commit_diff_hashes,
                                            map<string, string>& index_diff_hashes,
                                            const fs::path project_base_path) {
    vector<string> commit_paths = convert_map_to_sorted_values(commit_diff_hashes);
    vector<string> index_paths = convert_map_to_sorted_values(index_diff_hashes);

    vector<string> modified_files;
    std::set_intersection(commit_paths.begin(), commit_paths.end(), index_paths.begin(),
                          index_paths.end(), std::back_inserter(modified_files));
    for (const auto & path : modified_files) {
        cout << "modified: " << path_relative_to_project(project_base_path, path) << endl;
    }

    vector<string> deleted_files;
    std::set_difference(commit_paths.begin(), commit_paths.end(), index_paths.begin(),
                        index_paths.end(), std::back_inserter(deleted_files));
    for (const auto & path : deleted_files) {
        cout << "deleted: " << path_relative_to_project(project_base_path, path) << endl;
    }

    vector<string> new_files;
    std::set_difference(index_paths.begin(), index_paths.end(), commit_paths.begin(),
                        commit_paths.end(), std::back_inserter(new_files));
    for (const auto & path : new_files) {
        cout << "new: " << path_relative_to_project(project_base_path, path) << endl;
    }
}

void git_status_index_vs_project() {
    fs::path project_base_path = repo_find(fs::current_path());
    fs::path git_path = project_base_path / ".cpp-git";
    Option<GitTree> option_index_tree = get_index_tree(git_path);
    Option<GitTree> option_head_tree = get_head_tree(git_path);
    set<string> index_leaf_hashes;

    cout << "---------------Files not yet staged------------" << endl;
    if (!option_index_tree.exists) {
        if (!option_head_tree.exists) {
            // pass in empty index leaf hash
            print_unstaged_project_files(project_base_path, index_leaf_hashes, git_path,
                                         project_base_path);
        } else {
            set<string> commit_leaf_hashes;
            get_leaf_hashes_of_tree(&option_head_tree.content, commit_leaf_hashes, git_path);
            print_unstaged_project_files(project_base_path, commit_leaf_hashes, git_path,
                                         project_base_path);
        }
    } else {
        get_leaf_hashes_of_tree(&option_index_tree.content, index_leaf_hashes, git_path);
        print_unstaged_project_files(project_base_path, index_leaf_hashes, git_path,
                                     project_base_path);
    }
}

void git_status_commit_index(void) {
    fs::path project_base_path = repo_find(fs::current_path());
    fs::path git_path = project_base_path / ".cpp-git";
    cout << "---------------Files staged for commit:------------" << endl;
    // EC: no index
    // EC : no head
    Option<GitTree> option_index_tree = get_index_tree(git_path);
    Option<GitTree> option_head_tree = get_head_tree(git_path);

    if (option_index_tree.exists) {
        if (!option_head_tree.exists) {
            set<string> delete_hashes;
            set<string> set_head_hashes;
            // Pass empty set_head_hashes so everything is new
            print_new_index_nodes_and_calc_delete(&option_index_tree.content, delete_hashes,
                                                  set_head_hashes, git_path);
        } else {
            map<string, string> head_leaf_hashes;
            map<string, string> commit_diff_hashes;
            map<string, string> index_diff_hashes;
            // COMMON CASE, both index and head trees exist
            // TODO: overload
            get_leaf_hashes_of_tree(&option_head_tree.content, head_leaf_hashes, project_base_path,
                                    git_path);
            commit_diff_hashes = head_leaf_hashes;
            walk_index_and_calc_set_differences(&option_index_tree.content, project_base_path,
                                                commit_diff_hashes, index_diff_hashes,
                                                head_leaf_hashes, git_path);

            split_into_deleted_modified_new(commit_diff_hashes, index_diff_hashes,
                                            project_base_path);
        }
    }
    // Otherwise there is nothing staged
}

static void delete_new_files(map<string, string>& commit_diff_hashes,
                             map<string, string>& working_diff_hashes, fs::path project_base_path,
                             bool remove) {
    vector<string> commit_paths = convert_map_to_sorted_values(commit_diff_hashes);
    vector<string> working_paths = convert_map_to_sorted_values(working_diff_hashes);

    vector<string> new_files;
    std::set_difference(working_paths.begin(), working_paths.end(), commit_paths.begin(),
                        commit_paths.end(), std::back_inserter(new_files));

    if (remove)
        std::cout << "Deleting These Files: " << std::endl;
    else
        std::cout << "Would Delete These Files: " << std::endl;

    for (const auto & path : new_files) {
        cout << path_relative_to_project(project_base_path, path) << endl;
        if (remove) fs::remove(path);
    }
}

static void walk_working_and_calc_set_differences(fs::path current_path,
                                                  map<string, string>& commit_diff_hashes,
                                                  map<string, string>& working_diff_hashes,
                                                  const map<string, string>& head_leaf_hashes,
                                                  const fs::path git_path) {
    for (const auto & entry : fs::directory_iterator(current_path)) {
        const fs::path & path = entry.path();
        if (is_git_repo(current_path)) {
            continue;
        }

        if (fs::is_regular_file(entry)) {
            // read in the file + get its hash
            string blob_hash = read_project_file_and_write_object(git_path, path);
            // see if it's in the head_leaf_hashes
            bool inside_commit_tree = is_in_set(head_leaf_hashes, blob_hash);

            if (inside_commit_tree) {
                // Case 1: in set -> same file as in commit tree
                commit_diff_hashes.erase(blob_hash);
            } else {
                // Case 2: not in set ->
                working_diff_hashes.insert({blob_hash, path});
            }
        } else if (fs::is_directory(path)) {
            // recurse
            walk_working_and_calc_set_differences(path, commit_diff_hashes, working_diff_hashes,
                                                  head_leaf_hashes, git_path);
        } else {
            throw std::runtime_error("cpp-git cannot handle this file");
        }
    }
}

void git_clean(bool remove) {
    fs::path project_base_path = repo_find(fs::current_path());
    fs::path git_path = project_base_path / ".cpp-git";
    Option<GitTree> option_head_tree = get_head_tree(git_path);

    if (option_head_tree.exists) {
        map<string, string> head_leaf_hashes;
        map<string, string> commit_diff_hashes;
        map<string, string> working_diff_hashes;

        get_leaf_hashes_of_tree(&option_head_tree.content, head_leaf_hashes, project_base_path,
                                git_path);
        commit_diff_hashes = head_leaf_hashes;
        walk_working_and_calc_set_differences(project_base_path, commit_diff_hashes,
                                              working_diff_hashes, head_leaf_hashes, git_path);

        delete_new_files(commit_diff_hashes, working_diff_hashes, project_base_path, remove);
    }
    // Otherwise shouldn't clean
    else {
        throw std::runtime_error("Cannot clean until a commit has been made");
    }
}

static string ref_resolve(const fs::path& path, bool return_file_path = false) {
    string data = read_file(path);
    // No content if it's an initial commit
    if (data.size() == 0) return "";
    if (data.rfind("ref: ", 0) == 0)
        return ref_resolve(repo_find(path) / ".cpp-git" / data.substr(5), return_file_path);
    else if (return_file_path)
        return path;
    else
        return data;
}

static unordered_map<string, string> ref_list(const fs::path& base_path) {
    unordered_map<string, string> ret;
    fs::path repo_base_path = repo_find(fs::current_path());
    fs::path ref_path = repo_base_path / ".cpp-git" / "refs";

    for (const auto & entry : fs::directory_iterator(base_path)) {
        const fs::path & cur_path = entry.path();
        //        cout << cur_path << endl;
        if (fs::is_directory(cur_path)) {
            unordered_map<string, string> cur_ret = ref_list(cur_path);
            ret.insert(cur_ret.begin(), cur_ret.end());
        } else
            ret[cur_path.string().substr(ref_path.string().size() + 1)] = ref_resolve(cur_path);
    }
    return ret;
}

void git_show_ref(const string& prefix) {
    fs::path repo_base_path = repo_find(fs::current_path());
    fs::path ref_path = repo_base_path / ".cpp-git" / "refs";
    auto refs = ref_list(ref_path);
    for (auto it = refs.begin(); it != refs.end(); ++it)
        cout << it->second << " " + prefix + "/" << it->first << endl;
    cout << endl;
}

void cmd_show_ref(const vector<string>& args) { git_show_ref("refs"); }

void cmd_hash_object(const vector<string>& args) {
    if (args.size() <= 1 || CAT_FILE_SUBCMDS.find(args[0]) == CAT_FILE_SUBCMDS.end() ||
        args[0] == "--help") {
        throw std::runtime_error(HASH_OBJECT_USAGE);
    }
    git_hash_object(fs::canonical(args[1]), args[0]);
}

void git_hash_object(fs::path file_path, const string& fmt) {
    fs::path repo = repo_find(fs::current_path());
    string data = read_file(std::move(file_path));
    GitObject* obj = create_object(fmt, data, repo);
    write_object(obj, true);
}

void git_show_tag() {
    fs::path repo_base_path = repo_find(fs::current_path());
    fs::path tag_path = repo_base_path / ".cpp-git" / "refs" / "tags";
    auto refs = ref_list(tag_path);
    //for (auto it = refs.begin(); it != refs.end(); ++it){
    for (auto & it: refs){
        cout << it.first.substr(5) << endl;
    } 
    cout << endl;
}

void git_create_tag(string name, string object, bool if_create_object, string tag_message) {
    fs::path repo_base_path = repo_find(fs::current_path());
    fs::path tag_path = repo_base_path / ".cpp-git" / "refs" / "tags";

    string sha = object_find(repo_base_path, object, "commit");

    if (!if_create_object){
        write_file(tag_path / name, sha);
    }
    else {
        GitTag new_tag_obj = GitTag(repo_base_path / ".cpp-git", object, tag_message);
        string new_tag_hash = write_object(&new_tag_obj);
        write_file(tag_path / name, new_tag_hash);
    }
}

void cmd_tag(const vector<string>& args) {
    if (args.empty()) {
        // git tag: List all tags
        git_show_tag();
    } else if (args.size() == 1 || (args.size() == 2 && args[0] != "-a")) {
        // git tag NAME [OBJECT]: create a new lightweight tag NAME, pointing at HEAD (default) or
        // OBJECT
        string commit_hash;
        if (args.size() == 2){
            string commit_hash = args[1];
        }
        else {
            fs::path git_path = repo_find(fs::current_path()) / ".cpp-git";
            string current_full_branch_name = get_current_branch_full(git_path);
            commit_hash = get_commit_hash_from_branch(current_full_branch_name,
                                                      git_path);  // Default Value (HEAD)
        }
        git_create_tag(args[0], commit_hash, false);
    } else if (args[0] == "-a" && (args.size() >= 2)) {
        string commit_hash;
        string message = "";
        if (args.size() >= 3 && args[2] != "-m")
            commit_hash = args[2];  // NAME
        else {
            fs::path git_path = repo_find(fs::current_path()) / ".cpp-git";
            string current_full_branch_name = get_current_branch_full(git_path);
            commit_hash = get_commit_hash_from_branch(current_full_branch_name,
                                                      git_path);  // Default Value (HEAD)
        }
        if (args[args.size() - 2] == "-m") message = args.back();
        git_create_tag(args[1], commit_hash, true, message);
    } else {
        throw std::runtime_error("WRONG GIT TAG USAGE");
    }
}

void cmd_log(const vector<string>& args) {
    fs::path project_base_path = repo_find(fs::current_path());
    if (args.empty() || (args.size() == 2 && args[0] == "-n")) {
        int num = INT_MAX;
        if (args.size() > 0) num = std::stoi(args[1]);
        git_log(num);
    } else
        throw std::runtime_error(LOG_USAGE);
}

void git_log(int num) {
    fs::path project_base_path = repo_find(fs::current_path());
    fs::path git_path = project_base_path / ".cpp-git";

    string current_branch_name = get_current_branch_full(git_path);
    string current_commit_hash = get_commit_hash_from_branch(current_branch_name, git_path);

    if (current_commit_hash.empty()) return;

    do {
        GitCommit commit_obj;
        read_into_object(commit_obj, git_path, current_commit_hash);

        cout << "commit " + commit_obj.tree_hash << endl
             << commit_obj.commit_message << endl
             << endl;
        current_commit_hash = commit_obj.parent_hash;
        num -= 1;
    } while (!current_commit_hash.empty() and num > 0);
}

void cmd_status(const std::vector<std::string>& args) {
    git_status_commit_index();
    git_status_index_vs_project();
}

void cmd_clean(const std::vector<std::string>& args) {
    if (args.size() > 0) {
        if (args[0] == "-n") {
            git_clean(false);
        }
    } else {
        git_clean(true);
    }
}

void cmd_init(const vector<string>& args) {
    if ((args.size() >= 1 && args[0] == "--help") || args.size() > 1) {
        throw std::runtime_error(GIT_INIT_USAGE);
    }
    git_init(fs::current_path());
}

void cmd_add(const vector<string>& args) {
    if (args.size() != 1) {
        throw std::runtime_error("git add error. specify just the file or folder to add");
    }
    fs::path path = args[0];
    path = fs::canonical(path);
    if (fs::is_directory(path)) {
        git_add_folder(path);
        /* std::cout << "calling add folder" << std::endl; */
    } else if (fs::is_regular_file(path)) {
        git_add_file(path);
        /* std::cout << "calling add file" << std::endl; */
    } else {
        throw std::runtime_error(
            "git add error. cannot handle this kind of file/specified file does not exist on "
            "filesystem");
    }
}

void cmd_cat_file(const vector<string>& args) {
    if (args.size() <= 1 || CAT_FILE_SUBCMDS.find(args[0]) == CAT_FILE_SUBCMDS.end() ||
        args[0] == "--help") {
        throw std::runtime_error(CAT_FILE_USAGE);
    }
    git_cat_file(fs::canonical(args[1]), args[0]);
}

void cmd_checkout(const vector<string>& args) {
    if (args.size() != 1) {
        throw std::runtime_error(CHECKOUT_USAGE);
    }
    std::string branch_or_file = args[0];
    fs::path git_path = repo_find(fs::current_path()) / ".cpp-git";
    if (fs::exists(git_path / get_full_branch_name(branch_or_file))) {
        git_checkout_branch(branch_or_file);
    } else if (fs::exists(fs::canonical(branch_or_file))) {
        git_checkout_file(fs::canonical(branch_or_file));
    } else {
        throw std::runtime_error(CHECKOUT_USAGE);
    }
}

void cmd_commit(const vector<string>& args) {
    if (args.size() != 2) {
        throw std::runtime_error("First option should be -m/--amend, second is the commit message");
    }
    if (args[0] == "-m"){
        git_commit(args[1]);
    }
    else if (args[0] == "--amend"){
        git_amend(args[1]);
    }
}

void cmd_reset(const vector<string>& args) {
    if (args.empty()) {
        git_reset_project(false);
    } else if (args[0] == "--mixed") {
        if (args.size() == 2) {
            git_reset_file(args[1], false);
        } else {
            git_reset_project(false);
        }
    } else if (args[0] == "--hard") {
        if (args.size() == 2) {
            git_reset_file(args[1], true);
        } else {
            git_reset_project(true);
        }
    } else{
        throw std::runtime_error("git reset error.");
    }
}

void cmd_branch(const vector<string>& args) {
    fs::path project_base_path = repo_find(fs::current_path());
    fs::path git_path = project_base_path / ".cpp-git";
    if (args.empty() || args[0] == "--list") {
        git_branch_list();
    } else if (args[0] == "-d" && args.size() == 2) {
        string branch_name = args[1];
        if (fs::exists(git_path / get_full_branch_name(branch_name))) {
            git_branch_delete(branch_name);
        } else {
            throw std::runtime_error("git branch delete error. This is not an existing branch name");
        }
    } else if (args.size() == 1) {
        std::string branch_name = args[0];
        if (!fs::exists(git_path / get_full_branch_name(branch_name))) {
            git_branch_new(branch_name);
        } else {
            throw std::runtime_error("git branch create error. This is already an existing branch name");
        }
    } else {
        throw std::runtime_error("invalid git branch usage");
    }
}
