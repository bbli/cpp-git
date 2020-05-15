# README
---

## How to compile project
Just your standard way to compile a project with cmake:
1. make a build directory
2. cd into build directory
3. run `cmake ..`(if build directory is beneath project root)
4. run `make`

## TODO
### Can be done in Isolation Right now
* add interpreter [DONE]
* Have interpreter check validity of command line arguments + package into data structures before sending off
* define destructor/rest of the constructors for GitObjects
* finish writing write_object_and_read_object tests for the rest of the GitObjects
* Add references/tags
* add lazy write optimization to write_object + write test for it [DONE]
* conversion between relative to absolute path in interpreter code [DONE]
* git checkout optimization
* make to_internal private function [DONE]
* seperate writing to index in read_project_folder_and_write_tree into separate function

* implement git update-ref
* fix memory leaks -- no delete after newing git object

* git reset --medium(aka default) = just change index file to point to HEAD commit [DONE]
* git reset --hard = do above + walkTreeAndReplace on HEAD commit [DONE]
* git log: walk HEAD backwards , printing as you go along
    * filter 
    * --stat = call "git status" between each commit
* git branch
    * -D branch_name: delete the branch
    * -a: show all branches
    * nothing: create a new branch and switch to it(Note this is different from default behavior)
* git cat-file = take the hash and try to find the file. If found, read it in using read_file and print the contents. Otherwise throw not found error
* git commit [DONE]

   
### Future Work
* make strings into binary??
* add compression
* clean up tests 
* clean up header files(in particular in commands.hpp. Too many function are being exported right now)


## External Libraries
* Testing from GoogleTest framework(https://github.com/google/googletest)
* Sha1 hash functionality(https://github.com/vog/sha1)
* Range-v3 library(https://github.com/ericniebler/range-v3)
