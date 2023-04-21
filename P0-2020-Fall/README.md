# OS-Internal-and-Design
4350 Projects
Dougie Townsell
townsell.4@wright.edu


Accomplished - I accomplished this project by working in shell.cpp directory.cpp and inode.cpp. In shell.cpp I implemented the hard links and softlinks. I also had to add some helper functions to support the links. Also I use those helper function in mv, cd and rm to acocunt for the link commands. In inode.cpp I adjusted the show method to account for the links and show in the link count 

Indirects - by far was the hardest part of the project I did not get that all the way done I got single done but double was to hard to do. I did not have enough time to get that done. 

# Design

`ln hardlink` - I used some helper functions to deal with the entries and deal with the path.  The functions cd, mv, cp, ls, and rm account for the hard links

`ln -s`  - For symbolic link I kept the same idea I used for hard link I used some helper functions to deal with the entries and deal with the path.  The functions cd, mv, cp, ls, and rm account for the soft links
 
`inode show` - I added some checks to account for the type of link and link count so it displays 

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
-rw-rw-r-- 1 dougie dougie   2983 Mar  7 00:28 README.md
-rw------- 1 dougie dougie  17262 Mar 26 17:52 shell.cpp
-rw------- 1 dougie dougie   3780 Apr  2  2007 simdisk.cpp
-rw-rw-r-- 1 dougie dougie    179 Mar  8 16:34 testscript.txt
-rw-r--r-- 1 dougie dougie   2718 Sep 24  2020 typescript
-rw------- 1 dougie dougie    112 Apr  2  2007 user.cpp
-rw------- 1 dougie dougie   7172 Mar 26 16:46 volume.cpp
`
