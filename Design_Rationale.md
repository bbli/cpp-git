### Design Decisions
* why we used the ranges library
* explain design decision
    * get_fmt?
    * tree refers to tree object, project refers to source files/working directory
    * index representation:
        - during error checking for git checkout,git add, and git commit, we would need to compare against head commit tree, since index will always refer to a tree, even if nothing is staged ->(For example, you shouldn't make a commit if you have nothing staged. But if index points to the latest commit tree, there is always something in there) -> amounts to an extra I/O operation to solve this ambiguity
        - when the repo is started and HEAD does not point to a commit object(wel technically master doesn't point to a commit object), we will need to initialize an empty tree and commit object so that the index will have a commit tree to latch on to. But git doesn't do this(show example of empty repo). And in fact, if you look at the index file, you will see that the head commit is not in there(show example too)
        - to me, this is a better representation given the immutability of git objects. Git does not take a copy of the HEAD tree and modify it. Rather, everytime you add to the index, you are actually creating a new tree, in which some of its nodes, in particular the tree nodes(both root and subtree kinds), will not be used in the final tree that gets commited.

### In Depth Documentation
* explain GIT internals
    * content addressable file system(one implication is lazy writes)
* explain differences with Git
    * format
    * commit: no committer, pgp signature, etc
    * no commands involving diffing:
        * rebase
        * merge
        * cherry-pick
    * new/modified files
    * algorithms may not be what git actually uses

* algorithms:
    * status:
        - copy of dictionary for rare edge case that you have two copies of the same file
        - new vs modified
        - deletes
    * add:
