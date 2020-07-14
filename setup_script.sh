sudo rm -rf *
#cmake -DCMAKE_BUILD_TYPE=asan ~/Dropbox/Code/Projects/cpp-git
cmake -DCMAKE_BUILD_TYPE=Debug ~/Dropbox/Code/Projects/cpp-git
make -j4
cd src
mkdir profile
cd profile
../cpg init
echo "apple" > apple.txt
../cpg add apple.txt
