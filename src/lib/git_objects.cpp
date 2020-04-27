#include "git_objects.hpp"
#include <range/v3/view.hpp>
#include <range/v3/range/conversion.hpp>
#include <helper.hpp>

GitObject::GitObject(fs::path git_path) {
    this->git_path = git_path;
    // implicity change to string
}
GitCommit::GitCommit(fs::path git_path, const std::string& data) : GitObject(git_path) {
    to_internal(data);
}

GitCommit::GitCommit(fs::path git_path, const std::string& tree_hash,
                const std::string parent_hash, const std::string commit_message):
                    GitObject(git_path),
                    tree_hash(tree_hash),
                    parent_hash(parent_hash),
                    commit_message(commit_message){};

GitCommit::GitCommit(fs::path git_path) : GitObject(git_path), parent_hash(""), commit_message("") {
    GitTree* tree_obj = new GitTree(git_path);
    std::string tree_obj_hash = write_object(tree_obj, true);
    this->tree_hash = tree_obj_hash;
    delete tree_obj;
}

GitTree::GitTree(fs::path git_path, const std::string& data) : GitObject(git_path) {
    to_internal(data);
}
GitTree::GitTree(fs::path git_path) : GitObject(git_path), directory(std::vector<GitTreeNode>{}){};
GitTag::GitTag(fs::path git_path, const std::string& data) : GitObject(git_path) {
    to_internal(data);
}
GitBlob::GitBlob(fs::path git_path, const std::string& data) : GitObject(git_path) {
    to_internal(data);
}

void GitCommit::to_internal(const std::string& data) {
    // extract tree_hash
    auto tree_hash_point = std::find_if(data.begin(), data.end(), [](auto x) { return x == '\n'; });
    if (tree_hash_point == data.end()) {
        throw "sommething wrong with commit file";
    }
    std::copy(data.begin(), tree_hash_point, std::back_inserter(tree_hash));
    /* std::cout << "Tree hash length: " << tree_hash.length() << std::endl; */
    // extract parent hash
    tree_hash_point = tree_hash_point + 1;
    auto parent_hash_point =
        std::find_if(tree_hash_point, data.end(), [](auto x) { return x == '\n'; });
    if (parent_hash_point == data.end()) {
        throw "sommething wrong with commit file";
    }
    std::copy(tree_hash_point, parent_hash_point, std::back_inserter(parent_hash));
    /* std::cout << "Parent hash length: " << parent_hash.length() << std::endl; */
    // extract commit_message
    parent_hash_point = parent_hash_point + 1;
    std::copy(parent_hash_point, data.end(), std::back_inserter(commit_message));
    /* std::cout << "Commit message length: " << commit_message.length() << std::endl; */
}

/* template<typename T> */
/* set_part(std::string &member, T range){ */
/*     std::transform() */
/* } */

std::ostream& operator<<(std::ostream& os, GitTreeNode& t) {
    os << "Name: " << t.name << " Type: " << t.type << " Hash: " << t.hash << std::endl;
    return os;
}

void GitTree::to_internal(const std::string& data) {
    using namespace ranges;
    std::vector<char> no_null(data.length() + 1);
    std::copy(data.begin(), data.end(), no_null.begin());
    /* std::cout << "Before remove:" << std::endl; */
    /* printer(no_null); */
    no_null.pop_back();
    /* std::cout << "After remove:" << std::endl; */
    /* printer(no_null); */

    auto entry_list = no_null | views::split('\n');
    for (auto entry : entry_list) {
        // extract the parts
        auto entry_parts = entry | views::split(' ');
        GitTreeNode tmp;
        for (auto [idx, part] : views::enumerate(entry_parts)) {
            if (idx == 0) {
                /* set_part(tmp.name,part); */
                /* tmp.name = std::string(part.begin(),part.end()); */
                tmp.type = to<std::string>(part);
            } else if (idx == 1) {
                /* set_part(tmp.type,part); */
                /* tmp.type = std::string(part.begin(),part.end()); */
                tmp.name = to<std::string>(part);
            } else if (idx == 2) {
                /* set_part(tmp.hash,part); */
                /* tmp.hash = std::string(part.begin(),part.end()); */
                tmp.hash = to<std::string>(part);
            } else {
                throw "should not have more than 3 parts";
            }
        }
        // update vector
        directory.push_back(tmp);
    }
    /* printer(directory); */
}
void GitTag::to_internal(const std::string& data) {
    // TODO
}
void GitBlob::to_internal(const std::string& data) { this->data = data; }

std::string GitCommit::to_filesystem(void) {
    return tree_hash + '\n' + parent_hash + '\n' + commit_message;
}
std::string GitTree::to_filesystem(void) {
    std::string data;
    if (directory.size()>0){
        for (auto node : directory) {
            data += node.type + ' ' + node.name + ' ' + node.hash + '\n';
        }
        // to account for extra '\n'
        data.pop_back();
    }
    else{
        data = "";
    }
    return data;
}
std::string GitTag::to_filesystem(void) { return "TODO"; }
std::string GitBlob::to_filesystem(void) { return data; }

std::string GitCommit::get_fmt(void) { return "commit"; }

std::string GitTree::get_fmt(void) { return "tree"; }

std::string GitTag::get_fmt(void) { return "tag"; }

std::string GitBlob::get_fmt(void) { return "blob"; }

void GitTree::add_entry(std::string type, std::string file_name, std::string hash) {
    if (type == "blob" || type == "tree") {
        directory.push_back({type, file_name, hash});
    } else {
        throw "Type should only be blob or tree";
    }
}
