## Design Decisions

### Git Objects
* Modeling of git objects with classes
* Our GitObjects had minimal methods, and instead most of our codebase are functions. One reason for this is that methods are often used to changed the internal state of any object, and git instead favors the functional style of creating new objects, as we shall see later
    * Indeed,(see picture of tree method)
* Actually, if we had split our read_object function into a seperate function for each object, we could have eliminate the use of the heap entirely, as given the functionality of git and how it internals work, we always know the type before we read
    * So runtime polymorphism is not needed
* why we used the ranges library
    * Picture of what we wanted to split
    * Without the ranges library, we would need to iterate through the file once to get all the markers for each line, and then once again to split each of the entries.
    * Or, we need to do a lot of iterator manipulating to parse in it one pass.
    * That said, perhaps more trouble than it was worth, because the error messages generate by the ranges library were rather long, even for C++ error messages. Also, I had to convert the input to a vector for the splitting to work, so there essentially was no speedup

* explain design decision
    * get_fmt?
    * tree refers to tree object, project refers to source files/working directory
    * index representation:
        - during error checking for git checkout,git add, and git commit, we would need to compare against head commit tree, since index will always refer to a tree, even if nothing is staged ->(For example, you shouldn't make a commit if you have nothing staged. But if index points to the latest commit tree, there is always something in there) -> amounts to an extra I/O operation to solve this ambiguity
        - when the repo is started and HEAD does not point to a commit object(wel technically master doesn't point to a commit object), we will need to initialize an empty tree and commit object so that the index will have a commit tree to latch on to. But git doesn't do this(show example of empty repo). And in fact, if you look at the index file, you will see that the head commit is not in there(show example too)
        - to me, this is a better representation given the immutability of git objects. Git does not take a copy of the HEAD tree and modify it. Rather, everytime you add to the index, you are actually creating a new tree, in which some of its nodes, in particular the tree nodes(both root and subtree kinds), will not be used in the final tree that gets commited.

### In Depth Documentation
* Data format for each object
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
