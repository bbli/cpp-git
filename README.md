# README
---

## How to compile project
1. make a build directory
2. cd into build directory
3. run `cmake ..`
4. run `make`

## TODO
### Can be done in Isolation Right now
* add interpreter [DONE]
* define destructor/rest of the constructors for GitObjects
* finish writing write_object_and_read_object tests for the rest of the GitObjects
* Add references/tags
* add lazy write optimization to write_object + write test for it
* Define TearDown to delete created files from testing
* conversion between relative to absolute path in interpreter code
* git_path speedup optimization
* implement git update-ref
* fix memory leaks -- no delete after newing git object

* git reset --medium(aka default) = just change index file to point to HEAD commit
* git reset --hard = do above + walkTreeAndReplace on HEAD commit
* git log: walk HEAD backwards , printing as you go along
    * filter 
    * --stat = call "git status" between each commit
* git branch
    * -D branch_name: delete the branch
    * -a: show all branches
    * nothing: create a new branch and switch to it(Note this is different from default behavior)
* git cat-file = take the hash and try to find the file. If found, read it in using read_file and print the contents. Otherwise throw not found error
* git commit

* command
    * cat-file [DONE]
    * init [DONE]
    * add
    * cat-file
    * checkout 
    * commit   
    * hash-object
    * init     
    * log      
    * ls-tree 
    * merge    
    * rebase   
    * rev-parse
    * rm  
    * show-ref
    * tag
   
### Future Work
* change throws to classes
* make strings into binary

### Presentation
* explain GIT internals
* explain design decision
* explain differences with Git
    * format
    * HEAD


## External Libraries
* Testing from GoogleTest framework(https://github.com/google/googletest)
* Sha1 hash functionality(https://github.com/vog/sha1)
* Range-v3 library(https://github.com/ericniebler/range-v3)
