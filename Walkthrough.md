
## Tutorial 
1. git init -> call `tree .cpp-git`
2. create 2 new files and a folder with one file -> call `cpg add .`
3. `cpg status`, commit, and `cpg status again`
3. then modify a file -> call `cpg status` to see unstaged
4. add it then call `cpg status` to see modified
5. commit, then call `cpg log`
6. delete a file, call `cpg add .` + `cpg status`
7. `cpg reset --hard` to get file back 
8. create new branch and `cpg checkout new`
9. add a commit to new_branch
10. call `cpg branch`
11. modify `a.txt`, checkout master with stuff inside index
12. call `cpg reset --hard` + `cpg checkout master` -> see that the file changed in new_branch has reverted
