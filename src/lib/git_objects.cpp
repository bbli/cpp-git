#include "git_objects.hpp"

#include <helper.hpp>
using namespace std;

GitObject::GitObject(fs::path git_path) {
    this->git_path = git_path;
    // implicity change to string
}

GitCommit::GitCommit(fs::path git_path, const string& data) : GitObject(git_path) {
    to_internal(data);
}

GitCommit::GitCommit(fs::path git_path, string tree_hash, string parent_hash,
                     string commit_message)
    : GitObject(git_path),
      tree_hash(std::move(tree_hash)),
      parent_hash(std::move(parent_hash)),
      commit_message(std::move(commit_message)){};

GitCommit::GitCommit(fs::path git_path) : GitObject(git_path), parent_hash(""), commit_message("") {
    GitTree tree_obj = GitTree(git_path);
    string tree_obj_hash = write_object(&tree_obj, true);
    this->tree_hash = tree_obj_hash;
}

GitTree::GitTree(fs::path git_path, const string& data) : GitObject(git_path) { to_internal(data); }

GitTree::GitTree(fs::path git_path) : GitObject(git_path), directory(vector<GitTreeNode>{}){};

GitTag::GitTag(fs::path git_path, const string& data) : GitObject(git_path) { to_internal(data); }

GitTag::GitTag(fs::path git_path, const string commit_hash, const string tag_message)
    : GitObject(git_path), commit_hash(std::move(commit_hash)), tag_message(std::move(tag_message)) {}

GitBlob::GitBlob(fs::path git_path, const string& data) : GitObject(git_path) { to_internal(data); }

void GitCommit::to_internal(const string& data) {
    // extract tree_hash
    auto tree_hash_point = find_if(data.begin(), data.end(), [](auto x) { return x == '\n'; });
    if (tree_hash_point == data.end()) {
        throw std::runtime_error("sommething wrong with commit file");
    }
    std::copy(data.begin(), tree_hash_point, back_inserter(tree_hash));
    /* cout << "Tree hash length: " << tree_hash.length() << endl; */
    // extract parent hash
    tree_hash_point = tree_hash_point + 1;
    auto parent_hash_point = find_if(tree_hash_point, data.end(), [](auto x) { return x == '\n'; });
    if (parent_hash_point == data.end()) {
        throw std::runtime_error("sommething wrong with commit file");
    }
    std::copy(tree_hash_point, parent_hash_point, back_inserter(parent_hash));
    /* cout << "Parent hash length: " << parent_hash.length() << endl; */
    // extract commit_message
    parent_hash_point = parent_hash_point + 1;
    std::copy(parent_hash_point, data.end(), back_inserter(commit_message));
    /* cout << "Commit message length: " << commit_message.length() << endl; */
}

/* template<typename T> */
/* set_part(string &member, T range){ */
/*     transform() */
/* } */
// TODO: unit test for this since throw concats
// ohh, concats implies throw is now a std::string rather than char*
static void check_find_error(typename string::const_iterator point,
                             typename string::const_iterator endpoint, string file_type) {
    if (point == endpoint) {
        throw std::runtime_error("something is wrong with this " + file_type + " file");
        /* throw std::runtime_error("something is wrong with this file"); */
    }
}

static typename string::const_iterator extract_string(typename string::const_iterator start_it,
                                                      typename string::const_iterator end_it,
                                                      string& node_part) {
    // Q: will this actually work/ I guess find_if deduces the type
    // with first two arguments, which then enforces the type of the
    // space_delimiter template
    auto space_delimiter = [](auto x) { return x == ' '; };

    // find copy endpoint
    auto endpoint_it = std::find_if(start_it, end_it, space_delimiter);
    check_find_error(endpoint_it, end_it, "tree");

    // copy range
    std::copy(start_it, endpoint_it, std::back_inserter(node_part));

    return endpoint_it;
}

static typename string::const_iterator extract_hash(typename string::const_iterator start_it,
                                                    typename string::const_iterator end_it,
                                                    string& node_part) {
    auto new_line_delimiter = [](auto x) { return x == '\n'; };

    // find copy endpoint
    auto endpoint_it = std::find_if(start_it, end_it, new_line_delimiter);

    // copy range
    std::copy(start_it, endpoint_it, std::back_inserter(node_part));

    return endpoint_it;
}

typename string::const_iterator GitTree::extract_one_tree_node(
    typename string::const_iterator start_it, const string& data) {
    GitTreeNode new_tree_node;
    string type;
    string name;
    string hash;

    // extract parts
    auto type_endpoint = extract_string(start_it, data.end(), type);
    auto name_endpoint = extract_string(++type_endpoint, data.end(), name);
    auto hash_endpoint = extract_hash(++name_endpoint, data.end(), hash);

    new_tree_node.type = type;
    new_tree_node.name = name;
    new_tree_node.hash = hash;

    this->directory.push_back(new_tree_node);
    return hash_endpoint;
}

ostream& operator<<(ostream& os, GitTreeNode& t) {
    os << "Name: " << t.name << " Type: " << t.type << " Hash: " << t.hash << endl;
    return os;
}

void GitTree::to_internal(const string& data) {
    typename string::const_iterator curr_it = data.begin();
    while (curr_it != data.end()) {
        curr_it = this->extract_one_tree_node(curr_it, data);
        if (curr_it == data.end()) {
            break;
        } else if (*curr_it != '\n') {
            throw std::runtime_error("parsing error");
        } else {
            curr_it++;
        }
    }
    /* std::cout << "To internal finished. Tree object is:" << std::endl; */
    /* printer(this->directory); */
}

/* void GitTree::to_internal(const string& data) { */
/*     using namespace ranges; */
/*     vector<char> no_null(data.length() + 1); */
/*     std::copy(data.begin(), data.end(), no_null.begin()); */
/*     /1* cout << "Before remove:" << endl; *1/ */
/*     /1* printer(no_null); *1/ */
/*     no_null.pop_back(); */
/*     /1* cout << "After remove:" << endl; *1/ */
/*     /1* printer(no_null); *1/ */

/*     auto entry_list = no_null | views::split('\n'); */
/*     for (auto entry : entry_list) { */
/*         // extract the parts */
/*         auto entry_parts = entry | views::split(' '); */
/*         GitTreeNode tmp; */
/*         for (auto [idx, part] : views::enumerate(entry_parts)) { */
/*             if (idx == 0) { */
/*                 /1* set_part(tmp.name,part); *1/ */
/*                 /1* tmp.name = string(part.begin(),part.end()); *1/ */
/*                 tmp.type = to<string>(part); */
/*             } else if (idx == 1) { */
/*                 /1* set_part(tmp.type,part); *1/ */
/*                 /1* tmp.type = string(part.begin(),part.end()); *1/ */
/*                 tmp.name = to<string>(part); */
/*             } else if (idx == 2) { */
/*                 /1* set_part(tmp.hash,part); *1/ */
/*                 /1* tmp.hash = string(part.begin(),part.end()); *1/ */
/*                 tmp.hash = to<string>(part); */
/*             } else { */
/*                 throw std::runtime_error("should not have more than 3 parts"); */
/*             } */
/*         } */
/*         // update vector */
/*         directory.push_back(tmp); */
/*     } */
/*     /1* printer(directory); *1/ */
/* } */

void GitTag::to_internal(const string& data) {
    auto commit_hash_point = find_if(data.begin(), data.end(), [](auto x) { return x == '\n'; });
    if (commit_hash_point == data.end()) {
        throw std::runtime_error("sommething wrong with tag file");
    }
    std::copy(data.begin(), commit_hash_point, back_inserter(commit_hash));
    std::copy(commit_hash_point + 1, data.end(), back_inserter(tag_message));
}

void GitBlob::to_internal(const string& data) { this->data = data; }

string GitCommit::to_filesystem(void) {
    return tree_hash + '\n' + parent_hash + '\n' + commit_message;
}
string GitTree::to_filesystem(void) {
    string data;
    if (directory.size() > 0) {
        for (const auto & node : directory) {
            data += node.type + ' ' + node.name + ' ' + node.hash + '\n';
        }
        // to account for extra '\n'
        data.pop_back();
    } else {
        data = "";
    }
    return data;
}
string GitTag::to_filesystem(void) { return commit_hash + '\n' + tag_message; }

string GitBlob::to_filesystem(void) { return data; }

string GitCommit::get_fmt(void) { return "commit"; }

string GitTree::get_fmt(void) { return "tree"; }

string GitTag::get_fmt(void) { return "tag"; }

string GitBlob::get_fmt(void) { return "blob"; }

void GitTree::add_entry(string type, string file_name, string hash) {
    if (type == "blob" || type == "tree") {
        directory.push_back({type, file_name, hash});
    } else {
        throw std::runtime_error("Type should only be blob or tree");
    }
}
