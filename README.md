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

* git reset --medium = just change index file to point to HEAD commit
* git reset --hard = do above + walkTreeAndReplace on HEAD commit

* implement git update-ref
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
* move all gitobject code to its own file
* move scratch in main exec to test.cpp
* change throws to classes
* make strings into binary

## External Libraries
* Testing from GoogleTest framework(https://github.com/google/googletest)
* Sha1 hash functionality(https://github.com/vog/sha1)
* Range-v3 library(https://github.com/ericniebler/range-v3)
