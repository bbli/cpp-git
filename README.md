# README
---
## How to compile project
Just your standard way to compile a project with cmake:
1. make a build directory
2. cd into build directory
3. run `cmake ..`(if build directory is beneath project root. Otherwise change `..` to the build directory path)
4. run `make && make install`(will be installed in "bin" directory in project root)

(to install tests, clone the repo recusively and set `DPACKAGE_TESTS=ON` when running cmake. Then either run `make test` or `test/TEST` if you want to see the actual outputs)

## Intro/Differences with Git
* No remote commands(such as `git pull`, `git push`, etc)
* No diffing commands(such as `git merge`, `git rebase`, `git cherry-pick`, etc)
* Data format and index representation
* Perhaps difference in algorithms -> idk since I never read the source code

But otherwise, most of the git commands have been implemented to their full specifications, though less common flags are not supported. Below is a list of implemented comands, and a short summary of the algorithm behind the nontrival ones.
### Commands with Explanations
* `cpg add [file]`: first resolves file_name to its absolute path. Then walks the index tree(or commit tree if nothing is staged yet), recreating the tree except for where that file is located, which gets replaced by the new hash for that file. But because git is content addressable, the nodes for all parent directories of this file will be changed too. Also note that nodes arn't actually recreated, but merely point to the same git object. Furthermore, this is not a full on tree recursion, but actually a linear recursion in disguise.
* `cpg add [.]` or `cpg add [folder]`: Same as above, except once we reach the desired location, call another recursive algorithm that will traverse the working tree rather than the index/commit, and build the subtree bottom-up style.
* `cpg status`: we will need to walk all three(commit,index,working) trees and extract the leaf hashes for them. Calculating the set differences between the commit and index tree will yield nodes that are deleted/modified and new/modified respectively(because as far as git is concerned, a new file and a modified file will both produce a new hash). So, to distinguish between deleted/modified/new files, we will need do set operations again, but this time with repsect to the paths instead of the hashes. By applying set intersection, set difference with respect to the commit hashes, and set differences with respect to the index hashes, this acutally will cleanly partitioned the hashes into modified, deleted, and new files respectively.
* `cpg commit -m "[commit message]"`: Takes the index tree, wraps it in a commit object, which has a pointer to the previous commit. Then updates the current branch to point to the new commit, and cleans out the index. Note HEAD doesn't change, as it technically points to branches rather than commits, so it is a "pointer to a commit pointer", so to say.
* `cpg commit --amend "[commit message]"`: Same as above, except we give it a pointer to the commit one back of the previous commit. So the previous commit gets deleted in a "linked list" style, in that we just change pointers rather than actually deleting.
* `cpg reset --mixed/--hard [file]`: Sorta the opposite of `cpg add`. Walk the index tree and recreate it, except once we reach the file, don't add it as a child to the parent node. If `--hard` flag is passed, also replace the working tree's version of the file with the commit tree's one. Once again a linear recursion
* `cpg reset --mixed/--hard`: Easier than resetting a file. Just clean out the index, so nothing is staged now. If `--hard` flag is passed, replace the entire working tree with the commit tree.
* `cpg clean` or `cpg clean -n`: Very similar to `cpg status`. This is because we want to find the new files in the working directory, as those are the one that are not under version control and should be deleted. Passing `-n` will just print the files that will be deleted, rather than actually deleting them.

### Remaining Commands
* `cpg init`
* `cpg log` or `cpg log -n [number]`
* `cpg branch` or `cpg branch --list`
* `cpg branch [new branch]`
* `cpg branch -d [branch to delete]`
* `cpg checkout [file]`
* `cpg checkout [branch]`
   

## Future Work
* Implement garbage collection `git gc` -> algorithm for this should be as follows. Keep a set of all hashes as you traversethe git DAG backwards/into each tree, with branches and tags as "sources". Afterwards, traverse the object directory and delete any file which is not in this set
* Create "null" object so all objects can be statically allocated -> As it turns out, we **do not need runtime polymorphism(as we always know the type before retrieving from the .cpp-git/objects directory)**. But when refactoring the codebase, it turns out acessing the objects through pointers makes error checking easier b/c of `nullptr`. So once we have a "null" object, we can keep this nice error checking while making sure there are no memory leaks.

* add compression of object files-> easy but didn't do yet for debugging reasons
* clean up tests -> some tests do not use the newer functions/are too hardcoded.  Also remove that one test fixture(as personally I now don't believe in the usefulness of test fixtures)
* clean up header files -> in particular in commands.hpp. Too many function are being exported right now
* delete git path member from all the objects -> currently can't because part of test code/`write_object` relies on it
* add context dictionary for optimizations/less arguments passed to functions -> for example, sometimes the code logic will call `get_head_tree` twice, which amounts to 2 I/Os
* make algorithms account for same content in different file case -> as of now, most algorithms assume the content in each file of the repository is unique


## External Libraries
* Testing from GoogleTest framework(https://github.com/google/googletest)
* Sha1 hash functionality(https://github.com/vog/sha1)
