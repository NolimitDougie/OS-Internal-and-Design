# OS-Internal-and-Design
4350 Projects
Dougie Townsell
townsell.4@wright.edu


Accomplished - In shell.cpp I implmented background, redirection, pipe in the main function. Also I added some more functions in shell.cpp to complete the project. 



# Design

Background -  I used pthreads to implement background I created two void functions one for shell command and one for system command. The function for the  On pthread_create I use those functions as the 3rd paramter and pass in buf + 1 on the system command and a copy of buf for the shell command.

Redirection - I created a seperateSpaces function to take in a character pointer to strip any spaces
I used strchr to search buf for the `>` passed in c+1 into seperateSpaces and stored that in char pointer redir. From there I used my fd and make my creat system call and pass in redir with the file permission then make some dup calls 


Pipe -

`-rw-rw-r-- 1 dougie dougie    763 Mar  6 00:23 answers.txt`

`-rw------- 1 dougie dougie   2902 Apr  2  2007 bitvector.cpp`

`-rw-rw-r-- 1 dougie dougie  23568 Mar  6 18:51 bitvector.o`

`-rw------- 1 dougie dougie  65536 Mar  6 20:37 D1.dsk`

`-rw------- 1 dougie dougie   5352 Oct 30  2020 directory.cpp`

`-rw-rw-r-- 1 dougie dougie  32072 Mar  6 18:51 directory.o`

`-rw------- 1 dougie dougie    174 Apr  2  2007 diskParams.dat`

`-rw------- 1 dougie dougie   4199 Apr  2  2007 file.cpp`

`-rw-rw-r-- 1 dougie dougie  26520 Mar  6 18:51 file.o`

`-rw------- 1 dougie dougie   7000 Apr  2  2007 fs33types.hpp`

`-rw-rw-r-- 1 dougie dougie  74477 Mar  6 16:34 gdbDebugger.txt`

`-rw------- 1 dougie dougie   7569 Apr  2  2007 inodes.cpp`

`-rw-rw-r-- 1 dougie dougie  33608 Mar  6 18:51 inodes.o`

`-rw------- 1 dougie dougie    722 Jul  7  2020 Makefile`

`-rw------- 1 dougie dougie   3641 Apr  2  2007 mount.cpp`

`-rw-rw-r-- 1 dougie dougie  31680 Mar  6 18:51 mount.o`

`-rwxrwxr-x 1 dougie dougie 140976 Mar  6 20:37 P0`

`-rw------- 1 dougie dougie  13472 Mar  6 20:48 shell.cpp`

`-rw-rw-r-- 1 dougie dougie  73040 Mar  6 20:37 shell.o`

`-rw------- 1 dougie dougie   3780 Apr  2  2007 simdisk.cpp`

`-rw-rw-r-- 1 dougie dougie  31240 Mar  6 18:51 simdisk.o`

`-rw-r--r-- 1 dougie dougie   2718 Sep 24  2020 typescript`

`-rw------- 1 dougie dougie    112 Apr  2  2007 user.cpp`

`-rw------- 1 dougie dougie   6808 Apr  2  2007 volume.cpp`

`-rw-rw-r-- 1 dougie dougie  35864 Mar  6 18:51 volume.o`
