# README
---

## Differences with Git

## How to compile project
Just your standard way to compile a project with cmake:
1. make a build directory
2. cd into build directory
3. run `cmake ..`(if build directory is beneath project root)
4. run `make`

## Implemented Commands
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
* git reset needs to change current branch/fix git reset tmrw
    * git reset on file removes just that from index/recover if hard
* git clean
    * -n
    * -f
* git amend

   
## Future Work
* make strings into binary??
* add compression of object files-> easy but didn't do yet for debugging reasons
* clean up tests -> some tests do not use the newer functions/are too hardcoded.  Also remove that one test fixture(as personally I now don't believe in the usefulness of test fixtures)
* clean up header files -> in particular in commands.hpp. Too many function are being exported right now
* delete git path member from all the objects -> currently can't because part of test code/`write_object` relies on it
* add context dictionary for optimizations/less arguments passed to functions -> for example, sometimes the code logic will call `get_head_tree` twice, which amounts to 2 I/Os
* make algorithms account for same content in different file case
* implement garbage collection `git gc`: keep a set of all hashes as you traversethe git DAG backwards/into each tree, with branches and tags as "sources". Afterwards, traverse the object directory and delete any file which is not in this set


## External Libraries
* Testing from GoogleTest framework(https://github.com/google/googletest)
* Sha1 hash functionality(https://github.com/vog/sha1)
