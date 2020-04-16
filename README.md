# README
---

## How to compile project
1. make a build directory
2. cd into build directory
3. run `cmake ..`
4. run `make`

## TODO
### Can be done in Isolation Right now
* add interpreter
* define destructor/rest of the constructors for GitObjects
* finish writing writeObject_and_readObject tests for the rest of the GitObjects

### Future Work
* move all gitobject code to its own file
* move scratch in main exec to test.cpp
* change throws to classes
* make strings into binary

## External Libraries
* Testing from GoogleTest framework(https://github.com/google/googletest)
* Sha1 hash functionality(https://github.com/vog/sha1)
* Range-v3 library(https://github.com/ericniebler/range-v3)
