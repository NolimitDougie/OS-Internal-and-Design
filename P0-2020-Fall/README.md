# OS-Internal-and-Design
4350 Projects
Dougie Townsell
townsell.4@wright.edu


Accomplished - I accomplished this project by working in shell.cpp directory.cpp and volume.cpp. In shell.cpp I implemented the mkdir, cd, pwd, and mv command. I also had to add some functions and ajust some functions in directory.cpp and volume.cpp. The mv command was the hardest part of the project there are may cases to account for in the method 

# Design

mkdir - This took one line to create the directory `wd->createFile((byte *)a[0].s, wd->nInode)` I used the wd to call the create file and pass in the argument and wd Inode number.

cd - For cd I made the wd the fv->root. Then I looked for the '/' in the path and moved directorys using wd and prev a variable I created in the function 

rm - For improving rm I made a fucntion to check and see if the directory is empty. From the rm function I created a directory and called my function and passed in the counter variable and if the directory was empty  then we go head and call wd->delete file. If not then print out a message saying the directory was not deleted

mv - 

inode - 

`-rw------- 1 dougie dougie   2902 Apr  2  2007 bitvector.cpp
-rw-rw-r-- 1 dougie dougie 614400 Mar  6 17:48 cmdtries.txt
-rw------- 1 dougie dougie   6397 Mar 26 16:51 directory.cpp
-rw------- 1 dougie dougie    174 Apr  2  2007 diskParams.dat
-rw------- 1 dougie dougie   4199 Apr  2  2007 file.cpp
-rw------- 1 dougie dougie   7100 Mar 25 23:34 fs33types.hpp
-rw-rw-r-- 1 dougie dougie  74477 Mar  6 16:34 gdbSession.txt
-rw------- 1 dougie dougie   7569 Apr  2  2007 inodes.cpp
-rw------- 1 dougie dougie    722 Jul  7  2020 Makefile
-rw------- 1 dougie dougie   3641 Apr  2  2007 mount.cpp
-rw-rw-r-- 1 dougie dougie   2183 Mar 20 18:00 Project2answers.txt
-rw-rw-r-- 1 dougie dougie   2983 Mar  7 00:28 README.md
-rw------- 1 dougie dougie  17262 Mar 26 17:52 shell.cpp
-rw------- 1 dougie dougie   3780 Apr  2  2007 simdisk.cpp
-rw-rw-r-- 1 dougie dougie    179 Mar  8 16:34 testscript.txt
-rw-r--r-- 1 dougie dougie   2718 Sep 24  2020 typescript
-rw------- 1 dougie dougie    112 Apr  2  2007 user.cpp
-rw------- 1 dougie dougie   7172 Mar 26 16:46 volume.cpp
`
