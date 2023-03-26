# OS-Internal-and-Design
4350 Projects
Dougie Townsell
townsell.4@wright.edu


Accomplished - In shell.cpp I implmented background, redirection, pipe in the main function. Also I added some more functions in shell.cpp to complete the project. I encourted problems with multiple pipes I feel like I got very close but cant figure out the why its not outputting. Also I encountered some issues with a signle piple but was able to fix some dup2 and close calls and got the single pipe working.

# Design

Background -  I used pthreads to implement background I created two void functions one for shell command and one for system command. The function for the  On pthread_create I use those functions as the 3rd paramter and pass in buf + 1 on the system command and a copy of buf for the shell command.

Redirection - I created a seperateSpaces function to take in a character pointer to strip any spaces
I used strchr to search buf for the `>` passed in c+1 into seperateSpaces and stored that in char pointer redir. From there I used my fd and make my creat system call and pass in redir with the file permission then make some dup calls 


Pipe - I used strchr to search buf for the pipe. From there I created a file descriptor and pid and piped on the file descriptor then forked a new process and went through some checks on the pid and if the the pipe was succesful I went it and started doing dup2 calls and storing STDIN and STDOUT passing it to the next commands  

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
