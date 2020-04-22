#include "git_objects.hpp"
#include "helper.hpp"
#include "commands.hpp"

#include <filesystem>
#include <iostream>
#include <vector>
#include <unordered_map>


int test_function(void) {
    std::vector<int> test;
    std::cout << "Hello World" << std::endl;
    return 0;
}

void git_init(fs::path project_base_path) {
    // check that .git doesn't exist or is empty directory
    fs::path git_path = project_base_path / ".cpp-git";
    if (fs::exists(git_path)) {
        throw "init error: Not an empty path";
    }
    // initialize a repo object
    /* Repo repo(source_path,true); */

    // create object dir
    fs::create_directories(git_path / "objects");
    // create HEAD file with "ref: refs/heads/master"
    write_file(git_path / "HEAD", "ref: refs/heads/master");
    // create branches dir
    fs::create_directories(git_path / "branches");
    // create refs dir with tags+heads subdirectory
    fs::create_directories(git_path / "refs" / "tags");
    fs::create_directories(git_path / "refs" / "heads");
}


std::string getSubTreeHashForNewFile(std::string old_tree_hash, typename fs::path::iterator file_it,
                                const typename fs::path::iterator end_it, const fs::path git_path,
                                const fs::path& file_path) {
    // New Tree Object we are creating
    GitTree tree_obj(git_path);
    // Old Tree Object we are iterating over
    GitTree* old_tree_obj = dynamic_cast<GitTree*>(readObject(git_path, old_tree_hash));
    std::cout << "Currently at: " << *file_it << std::endl;
    bool found = false;
    for (auto node : old_tree_obj->directory) {
        // Case 1: Same branch as file
        if (checkNodeName(node,*file_it)) {
            // SubCase 1: file_it is at last element
            bool end = endOfPath(file_it,end_it);
            if (end) {
                // create GitBlob object, write to .git, then add to tree obj
                std::string blob_hash = readProjectFileAndWriteObject(git_path, file_path);
                tree_obj.add_entry(node.type, node.name, blob_hash);
                found = true;
            }
            // SubCase 2 : still haven't reached file
            else {
                file_it++;
                checkIfTree(node);
                std::string tree_hash =
                    getSubTreeHashForNewFile(node.hash, file_it, end_it, git_path, file_path);
                tree_obj.add_entry("tree", node.name, tree_hash);
            }
        }
        // Case 2: Not what we want to add to index
        else {
            tree_obj.add_entry(node.type, node.name, node.hash);
        }
    }
    // EC: if at end and we haven't found the file in the old tree directory
    if (endOfPath(file_it,end_it) && !found) {
        std::string blob_hash = readProjectFileAndWriteObject(git_path, file_path);
        tree_obj.add_entry("blob", file_path.filename(), blob_hash);
    }
    // Write Tree and return hash
    return writeObject(&tree_obj);
}

#if 1
// NOTE: can add new files, but they must be in existing folders already
std::string git_add_file(const fs::path& file_path) {
    fs::path project_base_path = repo_find(file_path);
    fs::path git_path = project_base_path / ".cpp-git";
    GitTree tree_obj(git_path);

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
    std::string root_tree_hash = getTreeHashOfIndex(git_path);
    std::cout << "HashOfHead:" << root_tree_hash << std::endl;
    std::string new_tree_hash =
        getSubTreeHashForNewFile(root_tree_hash, file_it, file_path.end(), git_path, file_path);

    std::cout << "Should write to index now" << std::endl;
    write_file(git_path / "index", new_tree_hash);
    return new_tree_hash;
}
#endif

std::string getSubTreeHashForNewFolder(std::string old_tree_hash, typename fs::path::iterator file_it,
                                  typename fs::path::iterator end_it, const fs::path git_path,
                                  const fs::path folder_path) {
    // New Tree Object we are creating
    GitTree tree_obj(git_path);
    // Old Tree Object we are iterating over
    GitTree* old_tree_obj = dynamic_cast<GitTree*>(readObject(git_path, old_tree_hash));
    std::cout << "Currently at: " << *file_it << std::endl;
    bool found = false;
    for (auto node : old_tree_obj->directory) {
        // Case 1: Same branch as file
        if (checkNodeName(node,*file_it)) {
            // SubCase 1: node points to adding folder
            bool end = endOfPath(file_it,end_it);
            if (end) {
                checkIfTree(node);
                std::string subtree_hash = readProjectFolderAndWriteTree(folder_path, false);
                // add to new_tree
                tree_obj.add_entry(node.type, node.name, subtree_hash);
                found = true;
            }
            // SubCase 2 : still haven't reached folder
            else {
                file_it++;
                checkIfTree(node);
                std::string tree_hash =
                    getSubTreeHashForNewFolder(node.hash, file_it, end_it, git_path, folder_path);
                tree_obj.add_entry("tree", node.name, tree_hash);
            }
        }
        // Case 2: Not what we want to add to index
        else {
            tree_obj.add_entry(node.type, node.name, node.hash);
        }
    }
    // EC: if at end and we haven't found the file in the old tree directory
    if (endOfPath(file_it,end_it) && !found) {
        std::string subtree_hash = readProjectFolderAndWriteTree(folder_path, false);
        // add to new_tree
        tree_obj.add_entry("blob", folder_path.filename(), subtree_hash);
    }
    // Write Tree and return hash
    return writeObject(&tree_obj);
}

std::string git_add_folder(const fs::path folder_path) {
    fs::path project_base_path = repo_find(folder_path);
    fs::path git_path = project_base_path / ".cpp-git";
    GitTree tree_obj(git_path);

    // Run file_it
    auto base_it = project_base_path.begin();
    auto file_it = folder_path.begin();
    while (base_it != project_base_path.end()) {
        base_it++;
        file_it++;
    }
    // *file_it=folder in /project_base_path/folder/etc...
    // Paths are unique, but individual names arn't
    // EC: if file is one_level
    // EC: if file is new file
    std::string old_tree_hash = getTreeHashOfIndex(git_path);
    std::cout << "HashOfHead:" << old_tree_hash << std::endl;
    std::string new_tree_hash =
        getSubTreeHashForNewFolder(old_tree_hash, file_it, folder_path.end(), git_path, folder_path);

    std::cout << "Should write to index now" << std::endl;
    write_file(git_path / "index", new_tree_hash);
    return new_tree_hash;
}

std::string readProjectFolderAndWriteTree(const fs::path& adding_directory, bool index) {
    fs::path project_base_path = repo_find(adding_directory);
    fs::path git_path = project_base_path / ".cpp-git";

    GitTree tree_obj(git_path);
    for (auto entry : fs::directory_iterator(adding_directory)) {
        fs::path path = entry.path();
        if (isGitRepo(path)) {
            continue;
        }
        std::cout << "Entry path: " << path << std::endl;
        if (fs::is_regular_file(entry)) {
            std::string blob_hash = readProjectFileAndWriteObject(git_path, path);
            // append to tree object
            tree_obj.add_entry("blob", path.filename(), blob_hash);
        } else if (fs::is_directory(path)) {
            std::string tree_hash = readProjectFolderAndWriteTree(path);
            tree_obj.add_entry("tree", path.filename(), tree_hash);
        } else {
            throw "cpp-git cannot handle this file";
        }
    }
    std::string output = writeObject(&tree_obj);
    if (index) {
        std::cout << "Should write to index now" << std::endl;
        write_file(git_path / "index", output);
    }
    return output;
}

std::string refResolve(fs::path path) {
    std::string data = read_file(path);
    if (data.rfind("ref: ", 0) == 0)
        return refResolve(data.substr(5));
    else
        return data;
}

std::unordered_map<std::string, std::string> refList(fs::path project_base_path) {
    fs::path ref_path = project_base_path / ".cpp-git" / "refs";
    std::unordered_map<std::string, std::string> ret;

    for (auto entry : fs::directory_iterator(ref_path)) {
        fs::path cur_path = entry.path();
        if (fs::is_directory(cur_path)) {
            std::unordered_map<std::string, std::string> cur_ret = refList(cur_path);
            ret.insert(cur_ret.begin(), cur_ret.end());
        }
        else
            ret[cur_path.string()] = refResolve(cur_path);
    }
    return ret;
}

void git_show_ref() {
    fs::path repo_base_path = repo_find(fs::current_path());
    auto refs = refList(repo_base_path);
    for ( auto it = refs.begin(); it != refs.end(); ++it )
        std::cout << it->second << " refs/" << it->first << std::endl;
    std::cout << std::endl;
}