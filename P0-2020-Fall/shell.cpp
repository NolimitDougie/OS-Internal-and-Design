/*
 * shell.C -- CEG433 File Sys Project shell
 * pmateti@wright.edu
 */
#include <sys/wait.h>
#include "fs33types.hpp"
#include <pthread.h>

extern MountEntry *mtab;
extern VNIN cwdVNIN;
FileVolume *fv; // Suspicious!
Directory *wd;  // Suspicious!

#define nArgsMax 10
char types[1 + nArgsMax]; // +1 for \0

/* An Arg-ument for one of our commands is either a "word" (a null
 * terminated string), or an unsigned integer.  We store both
 * representations of the argument. */

class Arg
{
public:
  char *s;
  uint u;
} arg[nArgsMax];

uint nArgs = 0;

uint TODO()
{
  printf("to be done!\n");
  return 0;
}

uint TODO(char *c)
{
  printf("%s to be done!\n", c);
  return 0;
}

uint isDigit(char c)
{
  return '0' <= c && c <= '9';
}

uint isAlphaNumDot(char c)
{
  return c == '.' || 'a' <= c && c <= 'z' || 'A' <= c && c <= 'Z' || '0' <= c && c <= '9';
}

int toNum(const char *c)
{
  return (c != 0 && '0' <= *c && *c <= '9' ? atoi(c) : 0);
}

SimDisk *mkSimDisk(byte *name)
{
  SimDisk *simDisk = new SimDisk(name, 0);
  if (simDisk->nSectorsPerDisk == 0)
  {
    printf("Failed to find/create simDisk named %s\n", name);
    delete simDisk;
    simDisk = 0;
  }
  return simDisk;
}

void doMakeDisk(Arg *a)
{
  SimDisk *simDisk = mkSimDisk((byte *)a[0].s);
  if (simDisk == 0)
    return;
  printf("new SimDisk(%s) = %c, nSectorsPerDisk=%d,"
         "nBytesPerSector=%d, simDiskNum=%d)\n",
         simDisk->name, (void *)simDisk, simDisk->nSectorsPerDisk,
         simDisk->nBytesPerSector, simDisk->simDiskNum);
  delete simDisk;
}

void doWriteDisk(Arg *a)
{
  SimDisk *simDisk = mkSimDisk((byte *)a[0].s);
  if (simDisk == 0)
    return;
  char *st = a[2].s; // arbitrary word
  if (st == 0)       // if it is NULL, we use ...
    st = "CEG433/633/Mateti";
  char buf[1024]; // assuming nBytesPerSectorMAX < 1024
  for (uint m = strlen(st), n = 0; n < 1024 - m; n += m)
    memcpy(buf + n, st, m); // fill with several copies of st
  uint r = simDisk->writeSector(a[1].u, (byte *)buf);
  printf("write433disk(%d, %s...) == %d to Disk %s\n", a[1].u, st, r, a[0].s);
  delete simDisk;
}

void doReadDisk(Arg *a)
{
  SimDisk *simDisk = mkSimDisk((byte *)a[0].s);
  if (simDisk == 0)
    return;
  char buf[1024]; // assuming nBytesPerSectorMAX < 1024
  uint r = simDisk->readSector(a[1].u, (byte *)buf);
  buf[10] = 0; // sentinel
  printf("read433disk(%d, %s...) = %d from Disk %s\n", a[1].u, buf, r, a[0].s);
  delete simDisk;
}

void doQuit(Arg *a)
{
  exit(0);
}

void doEcho(Arg *a)
{
  printf("%s#%d, %s#%d, %s#%d, %s#%d\n", a[0].s, a[0].u,
         a[1].s, a[1].u, a[2].s, a[2].u, a[3].s, a[3].u);
}

void doMakeFV(Arg *a)
{
  SimDisk *simDisk = mkSimDisk((byte *)a[0].s);
  if (simDisk == 0)
    return;
  fv = simDisk->make33fv();
  printf("make33fv() = %c, Name == %s, Disk# == %d\n",
         (void *)fv, a[0].s, simDisk->simDiskNum);

  if (fv)
  {
    wd = new Directory(fv, 1, 0);
    cwdVNIN = mkVNIN(simDisk->simDiskNum, 1);
  }
}

void doCopyTo(byte *from, byte *to)
{
  uint r = fv->write33file(to, from);
  printf("write33file(%s, %s) == %d\n", to, from, r);
}

void doCopyFrom(byte *from, byte *to)
{
  uint r = fv->read33file(to, from);
  printf("read33file(%s, %s) == %d\n", to, from, r);
}

void doCopy33(byte *from, byte *to)
{
  uint r = fv->copy33file(to, from);
  printf("copy33file(%s, %s) == %d\n", to, from, r);
}

void doCopy(Arg *a)
{
  byte *to = (byte *)a[0].s;
  byte *from = (byte *)a[1].s;

  if (a[0].s[0] == '@' && a[1].s[0] != '@')
  {
    doCopyTo(from, (to + 1));
  }
  else if (a[0].s[0] != '@' && a[1].s[0] == '@')
  {
    doCopyFrom((from + 1), to);
  }
  else if (a[0].s[0] != '@' && a[1].s[0] != '@')
  {
    doCopy33(from, to);
  }
  else
  {
    puts("Wrong arguments to cp.");
  }
}

void doLsLong(Arg *a)
{
  unsigned inode = wd->iNumberOf((byte *)a[0].s);

  if (inode)
  {
    Directory *dir = new Directory(fv, inode, 0);
    printf("\nDirectory listing for disk %s, cwdVNIN == 0x%0lx begins:\n",
           dir->fv->simDisk->name, (ulong)cwdVNIN);
    dir->ls(); // Suspicious!
    printf("Directory listing ends.\n");
    delete dir;
  }
  else
  {
    printf("\nDirectory listing for disk %s, cwdVNIN == 0x%0lx begins:\n",
           wd->fv->simDisk->name, (ulong)cwdVNIN);
    wd->ls(); // Suspicious!
    printf("Directory listing ends.\n");
  }
}

void lsRecursiveHelper(Directory *tmp)
{

  std::vector<std::string> entriesVec = tmp->getEntries();

  for (long unsigned int i = 0; i < entriesVec.size(); i++)
  {

    const char *tmpVec = entriesVec[i].c_str();
    uint inode = tmp->iNumberOf((byte *)tmpVec);

    if (inode != 0 && wd->fv->inodes.getType(inode) == iTypeDirectory)
    {
      Directory *newDir = new Directory(fv, inode, 0);
      lsRecursiveHelper(newDir);
    }

    printf("\nDirectory listing for disk %s, cwdVNIN == 0x%0lx begins:\n",
           tmp->fv->simDisk->name, (ulong)cwdVNIN);
    tmp->ls(); // Suspicious!
    printf("Directory listing ends.\n");
  }
  delete (tmp);
}

void lsRecursive(Arg *a)
{

  if (strcmp(a[0].s, "-lR") == 0 || strcmp(a[0].s, "-Rl") == 0)
  {
    uint inode = wd->iNumberOf((byte *)a[1].s);
    if (inode != 0 && fv->inodes.getType(inode) == iTypeDirectory)
    {
      Directory *curDir = new Directory(fv, inode, 0);
      lsRecursiveHelper(curDir);
    }
    wd->ls();
    return;
  }
  printf("Incorrect flag for recursive ls");
}

void doRm(Arg *a)
{
  char *dirName = (char *)a[0].s;
  byte *tmp = (byte *)dirName;
  uint inode = wd->iNumberOf(tmp);
  if (inode == 0)
  {
    printf("File/Directory not found.\n");
    return;
  }
  int numContents = 0;
  Directory *dir = new Directory(fv, inode, 0);

  bool remove = dir->isEmpty(numContents);

  if (wd->fv->inodes.getType(inode) == iTypeDirectory && remove)
  {
    wd->deleteFile(tmp, 1);
    printf("Successfully removed directory '%s' with inode %d and %d entries.\n", a[0].s, inode, numContents);
    return;
  }
  else if (wd->fv->inodes.getType(inode) == iTypeDirectory && !remove)
  {
    printf("Unable to remove directory '%s' with inode %d and %d entries.\n", a[0].s, inode, numContents);
    return;
  }
  else if (wd->fv->inodes.getType(inode) == iTypeOrdinary)
  {
    inode = wd->deleteFile((byte *)a[0].s, 1);
    printf("Successfully removed file '%s' with inode %d.\n", a[0].s, inode);
    return;
  }
}

void dorecursiveHelper(Directory *tmp)
{

  int count = 0;
  bool checkDir = tmp->isEmpty(count);

  if (!checkDir)
  {
    // Advance the directory
    std::vector<std::string> entriesVec = tmp->getEntries();
    for (long unsigned int i = 0; i < entriesVec.size(); i++)
    {

      const char *tmpVec = entriesVec[i].c_str();
      uint inode = tmp->iNumberOf((byte *)tmpVec);
      if (inode != 0 && wd->fv->inodes.getType(inode) == iTypeDirectory)
      {
        Directory *newDir = new Directory(fv, inode, 0);
        dorecursiveHelper(newDir);
      }
      tmp->deleteFile((byte *)tmp, 1);
    }
  }
  delete (tmp);
}

void dorecursiveRm(Arg *a)
{
  int count = 0;
  if (strcmp(a[0].s, "-fr") != 0 || strcmp(a[0].s, "-rf") != 0)
  {
    uint inode = wd->iNumberOf((byte *)a[1].s);
    if (inode != 0 && fv->inodes.getType(inode) == iTypeDirectory)
    {
      Directory *curDir = new Directory(fv, inode, 0);
      dorecursiveHelper(curDir);
    }
    wd->deleteFile((byte *)a[1].s, 1);
    return;
  }
  printf("Incorrect flag for recursive rm.\n");
}

void doInode(Arg *a)
{
  uint ni = a[0].u;
  char *name = a[0].s;
  uint inode = wd->iNumberOf((byte *)name);

  if (inode)
  {
    wd->fv->inodes.show(inode);
  }
  else if (ni)
    wd->fv->inodes.show(ni);
  else
    printf("%s doesn't exist\n", name);
}

void doMkDir(Arg *a)
{
  wd->createFile((byte *)a[0].s, wd->nInode);
}

void doPwd(Arg *a)
{
  unsigned int count = 0, inode, inodes[32];
  int j;
  Directory *dir;

  inode = wd->nInode;
  while (inode != fv->root->nInode)
  {
    inodes[count] = inode;
    count++;
    dir = new Directory(fv, inode, 0);
    inode = dir->iNumberOf((byte *)"..");
    delete dir;
  }
  if (count)
  {
    printf("/%s", fv->root->nameOf(inodes[count - 1]));
    for (j = count - 2; j >= 0; j--)
    {
      dir = new Directory(fv, inodes[j + 1], 0);
      printf("/%s", dir->nameOf(inodes[j]));
      delete dir;
    }
    printf("\n");
  }
  else
    printf("/\n");

} // end of PWD

void doChDir(Arg *a)
{
  char *path = (char *)a[0].s;
  char *c = path;
  c++;
  Directory *prevwd = wd;
  Directory *currentDir = prevwd->fv->root;

  if (path[0] == '/' && path[1] == '\0')
  {
    wd = prevwd->fv->root;
  }
  else if (path[0] == '/')
  {
    int i = 0;
    while (true)
    {

      if (c[i] == '/')
      {
        c[i] = '\0';
        int inode = currentDir->iNumberOf((byte *)c);
        if (inode)
        {
          currentDir = new Directory(prevwd->fv, inode, 0);
        }
        else
        {
          printf("Directory doesn't exist\n");
          break;
        }
        c = c + i + 1;
        i = 0;
        continue;
      }
      else if (c[i] == '\0')
      {
        int inode = currentDir->iNumberOf((byte *)c);
        if (inode)
        {
          currentDir = new Directory(prevwd->fv, inode, 0);
          wd = currentDir;
          doPwd(a);
        }
        break;
      }
      i++;
    } // end of while loop
      // eof if statement
  }
  else
  {
    int inode = prevwd->iNumberOf((byte *)path);
    if (inode)
    {
      currentDir = new Directory(prevwd->fv, inode, 0);
      wd = currentDir;
      doPwd(a);
    }
    else
    {
      printf("directory doesn't exist\n");
    }
  } // end of else block

} // end of function

uint findNumSlashes(char *path)
{
  char *findSlashes;
  uint numSlashes = 0;
  findSlashes = strchr(path, '/');
  while (findSlashes != NULL)
  {
    numSlashes++;
    findSlashes = strchr(findSlashes + 1, '/');
  }
  return numSlashes;
}

void updatePath(char *path, std::string newPath)
{
  for (long unsigned int i = 0; i < newPath.length(); i++)
  {
    path[i] = newPath[i];
  }
  path[newPath.length()] = '\0';
}

bool fixPath(char *path)
{
  for (int i = strlen(path) - 1; i >= 0; i--)
  {
    if (path[i] == '/')
    {
      path[i] = 0;
    }
    else
    {
      break;
    }
  }
  for (long unsigned int i = 0; i < strlen(path); i++)
  {
    if (path[i] == '/' && path[i + 1] == '/')
    {
      return false;
    }
  }
  return true;
}

std::vector<std::string> getPathVec(char *path)
{
  std::vector<std::string> pathVec;
  if (!fixPath(path))
  {
    return pathVec;
  }
  std::string remPathStr;
  if (path[0] != '/')
  {
    remPathStr = path;
    remPathStr = "/" + remPathStr;
    updatePath(path, remPathStr);
  }
  uint numSlashes = findNumSlashes(path);
  for (uint i = 0; i < numSlashes; i++)
  {
    pathVec.push_back(strtok(path, "/"));
    char *remPathChar = strtok(0, " \t");
    if (remPathChar == 0 && i == numSlashes - 1)
    {
      break;
    }
    remPathStr = remPathChar;
    remPathStr = "/" + remPathStr;
    updatePath(path, remPathStr);
  }
  return pathVec;
}

std::vector<std::string> doMvPath(char *path, bool &invalidPath, bool &IsFile, bool &exists)
{
  bool toRoot;
  bool afterRoot = true;
  Directory *startDir = wd;
  if (path[0] == '/')
  {
    toRoot = true;
    if (path[1] == 0)
    {
      afterRoot = false;
    }
    else if (path[1] == '/')
    {
      afterRoot = false;
      for (long unsigned int i = 0; i < strlen(path); i++)
      {
        if (path[i] != '/')
        {
          toRoot = false;
          break;
        }
        else
        {
          toRoot = true;
        }
      }
    }
  }
  if (toRoot)
  {
    Directory *childDir = wd;
    uint rootINode = 0;
    while (rootINode != 1)
    {
      rootINode = childDir->iNumberOf((byte *)"..");
      wd = new Directory(fv, rootINode, 0);
      if (childDir != startDir)
      {
        delete (childDir);
      }
      childDir = wd;
    }
  }
  std::vector<std::string> pathVec;
  if (afterRoot)
  {
    pathVec = getPathVec(path);
    uint iNode = 0;
    const char *pathEntry;
    for (long unsigned int i = 0; i < pathVec.size(); i++)
    {
      pathEntry = pathVec[i].c_str();
      iNode = wd->iNumberOf((byte *)pathEntry);
      if (iNode != 0 && wd->fv->inodes.getType(iNode) == iTypeDirectory)
      {
        if (i != pathVec.size() - 1)
        {
          Directory *tmp = wd;
          wd = new Directory(fv, iNode, 0);
          if (tmp != startDir)
          {
            delete (tmp);
          }
        }
      }
      else if (iNode != 0 && wd->fv->inodes.getType(iNode) == iTypeOrdinary && i == pathVec.size() - 1)
      {
        IsFile = true;
        break;
      }
      else if (iNode == 0 && i == pathVec.size() - 1)
      {
        exists = false;
      }
      else
      {
        invalidPath = true;
        break;
      }
    }
    if (pathVec.size() == 0)
    {
      invalidPath = true;
    }
  }
  return pathVec;
}

bool fileInPath(std::vector<std::string> pathVec, uint iNode)
{
  Directory *dir = wd;
  uint pathINode;
  const char *pathEntry;
  for (long unsigned int i = 0; i < pathVec.size(); i++)
  {
    pathEntry = pathVec[i].c_str();
    pathINode = dir->iNumberOf((byte *)pathEntry);
    if (pathINode == iNode)
    {
      if (dir != wd)
      {
        delete (dir);
      }
      return true;
    }
    Directory *tmp = dir;
    dir = new Directory(fv, pathINode, 0);
    if (tmp != wd)
    {
      delete (tmp);
    }
  }
  return false;
}

void doMv(Arg *a)
{
  Directory *startDir = wd;
  bool sourceInvalidPath = false;
  bool sourceIsFile = false;
  bool sourceExists = true;
  bool destInvalidPath = false;
  bool destIsFile = false;
  bool destExists = true;

  if (a[0].s[0] == '.')
  {
    printf("Cannot move '.' or '..'.\n");
    return;
  }
  char *destPath = new char[strlen(a[1].s) + 1];
  strcpy(destPath, a[1].s);

  std::vector<std::string> sourceVec = doMvPath(a[0].s, sourceInvalidPath, sourceIsFile, sourceExists);
  Directory *sourceDir = wd;
  wd = startDir;
  if (sourceVec.size() == 0)
  {
    delete (destPath);
    if (sourceDir != wd)
    {
      delete (sourceDir);
    }
    printf("Cannot move or rename root.\n");
    return;
  }
  std::vector<std::string> destVec = doMvPath(destPath, destInvalidPath, destIsFile, destExists);
  if (destVec.size() == 0)
  {
    destVec.push_back(".");
  }
  delete (destPath);
  Directory *destDir = wd;
  wd = startDir;

  if (!sourceExists || sourceInvalidPath || destInvalidPath || (destExists && destIsFile))
  {
    if (sourceDir != wd)
    {
      delete (sourceDir);
    }
    if (destDir != wd)
    {
      delete (destDir);
    }
    printf("Move/Rename failed.\n");
    return;
  }
  else if (!destExists)
  {
    uint flag;
    const char *sourceFile = sourceVec[sourceVec.size() - 1].c_str();
    const char *destName = destVec[destVec.size() - 1].c_str();
    uint iNode = sourceDir->iNumberOf((byte *)sourceFile);
    if (sourceIsFile)
    {
      flag = 0;
    }
    else
    {
      flag = 1;
      if (fileInPath(destVec, iNode))
      {
        printf("Cannot move a directory into its own subdirectory.\n");
        return;
      }
    }
    sourceDir->deleteFile((byte *)sourceFile, 0);
    destDir->customCreateFile((byte *)destName, iNode, flag);
    if (flag == 1)
    {
      Directory *newDir = new Directory(fv, iNode, 0);
      newDir->customDeleteFile((byte *)"..", 0);
      uint destINode = destDir->iNumberOf((byte *)".");
      newDir->customCreateFile((byte *)"..", destINode, flag);
      if (newDir != wd)
      {
        delete (newDir);
      }
    }
    printf("Renamed successfully.\n");
  }
  else if (destExists)
  {
    uint flag;
    const char *sourceFile = sourceVec[sourceVec.size() - 1].c_str();
    const char *destName = destVec[destVec.size() - 1].c_str();
    uint iNode = sourceDir->iNumberOf((byte *)sourceFile);
    uint destINode = destDir->iNumberOf((byte *)destName);
    Directory *tmp = destDir;
    destDir = new Directory(fv, destINode, 0);
    if (tmp != wd)
    {
      delete (tmp);
    }
    if (destDir->iNumberOf((byte *)sourceFile) != 0)
    {
      if (destDir != wd)
      {
        delete (destDir);
      }
      printf("File/Directory already exists.\n");
      return;
    }
    if (sourceIsFile)
    {
      flag = 0;
    }
    else
    {
      flag = 1;
      if (fileInPath(destVec, iNode))
      {
        if (destDir != wd)
        {
          delete (destDir);
        }
        printf("Cannot move a directory into its own subdirectory.\n");
        return;
      }
    }
    sourceDir->deleteFile((byte *)sourceFile, 0);
    destDir->customCreateFile((byte *)sourceFile, iNode, flag);
    if (flag == 1)
    {
      Directory *newDir = new Directory(fv, iNode, 0);
      newDir->customDeleteFile((byte *)"..", 0);
      newDir->customCreateFile((byte *)"..", destINode, flag);
      if (newDir != wd)
      {
        delete (newDir);
      }
    }
    printf("Moved successfully.\n");
  }

  if (sourceDir != wd)
  {
    delete (sourceDir);
  }
  if (destDir != wd)
  {
    delete (destDir);
  }
}

void doMountDF(Arg *a) // arg a ignored
{
  TODO("doMountDF");
}

void doMountUS(Arg *a)
{
  TODO("doMountUS");
}

void doUmount(Arg *a)
{
  TODO("doUmount");
}

/* The following describes one entry in our table of commands.  For
 * each cmmdName (a null terminated string), we specify the arguments
 * it requires by a sequence of letters.  The letter s stands for
 * "that argument should be a string", the letter u stands for "that
 * argument should be an unsigned int."  The data member (func) is a
 * pointer to the function in our code that implements that command.
 * globalsNeeded identifies whether we need a volume ("v"), a simdisk
 * ("d"), or a mount table ("m").  See invokeCmd() below for exact
 * details of how all these flags are interpreted.
 */

class CmdTable
{
public:
  char *cmdName;
  char *argsRequired;
  char *globalsNeeded; // need d==simDisk, v==cfv, m=mtab
  void (*func)(Arg *a);
} cmdTable[] = {
    {"cd", "s", "v", doChDir},
    {"cp", "ss", "v", doCopy},
    {"echo", "ssss", "", doEcho},
    {"inode", "u", "v", doInode},
    {"inode", "s", "v", doInode},
    {"ls", "", "v", doLsLong},
    {"ls", "s", "v", doLsLong},
    {"ls", "ss", "v", lsRecursive},
    {"lslong", "", "v", doLsLong},
    {"mkdir", "s", "v", doMkDir},
    {"mkdisk", "s", "", doMakeDisk},
    {"mkfs", "s", "", doMakeFV},
    {"mount", "us", "", doMountUS},
    {"mount", "", "", doMountDF},
    {"mv", "ss", "v", doMv},
    {"rddisk", "su", "", doReadDisk},
    {"rmdir", "s", "v", doRm},
    {"rm", "s", "v", doRm},
    {"rm", "ss", "v", dorecursiveRm},
    {"pwd", "", "v", doPwd},
    {"q", "", "", doQuit},
    {"quit", "", "", doQuit},
    {"umount", "u", "m", doUmount},
    {"wrdisk", "sus", "", doWriteDisk}};

uint ncmds = sizeof(cmdTable) / sizeof(CmdTable);

void usage()
{
  printf("The shell has only the following cmds:\n");
  for (uint i = 0; i < ncmds; i++)
    printf("\t%s\t%s\n", cmdTable[i].cmdName, cmdTable[i].argsRequired);
  printf("Start with ! to invoke a Unix shell cmd\n");
}

/* pre:: k >= 0, arg[] are set already;; post:: Check that args are
 * ok, and the needed simDisk or cfv exists before invoking the
 * appropriate action. */

void invokeCmd(int k, Arg *arg)
{
  uint ok = 1;
  if (cmdTable[k].globalsNeeded[0] == 'v' && cwdVNIN == 0)
  {
    ok = 0;
    printf("Cmd %s needs the cfv to be != 0.\n", cmdTable[k].cmdName);
  }
  else if (cmdTable[k].globalsNeeded[0] == 'm' && mtab == 0)
  {
    ok = 0;
    printf("Cmd %s needs the mtab to be != 0.\n", cmdTable[k].cmdName);
  }

  char *req = cmdTable[k].argsRequired;
  uint na = strlen(req);
  for (uint i = 0; i < na; i++)
  {
    if (req[i] == 's' && (arg[i].s == 0 || arg[i].s[0] == 0))
    {
      ok = 0;
      printf("arg #%d must be a non-empty string.\n", i);
    }
    if ((req[i] == 'u') && (arg[i].s == 0 || !isDigit(arg[i].s[0])))
    {
      ok = 0;
      printf("arg #%d (%s) must be a number.\n", i, arg[i].s);
    }
  }
  if (ok)
    (*cmdTable[k].func)(arg);
}

// Background

/* pre:: buf[] is the command line as typed by the user, nMax + 1 ==
 * sizeof(types);; post:: Parse the line, and set types[], arg[].s and
 * arg[].u fields.
 */

void setArgsGiven(char *buf, Arg *arg, char *types, uint nMax)
{
  for (uint i = 0; i < nMax; i++)
  {
    arg[i].s = 0;
    types[i] = 0;
  }
  types[nMax] = 0;

  strtok(buf, " \t\n"); // terminates the cmd name with a \0

  for (uint i = 0; i < nMax;)
  {
    char *q = strtok(0, " \t");
    if (q == 0 || *q == 0)
      break;
    arg[i].s = q;
    arg[i].u = toNum(q);
    types[i] = isDigit(*q) ? 'u' : 's';
    nArgs = ++i;
  }
}
/* pre:: name pts to the command token, argtypes[] is a string of
 * 's'/'u' indicating the types of arguments the user gave;; post::
 * Find the row number of the (possibly overloaded) cmd given in
 * name[].  Return this number if found; return -1 otherwise. */
int findCmd(char *name, char *argtypes)
{
  for (uint i = 0; i < ncmds; i++)
  {
    if (strcmp(name, cmdTable[i].cmdName) == 0 && strcmp(argtypes, cmdTable[i].argsRequired) == 0)
    {
      return i;
    }
  }
  return -1;
}
void *cmdline(char *buf)
{
  if (buf[0] == '!') // begins with !, execute it as
    system(buf + 1); // a normal shell cmd
  else
  {
    setArgsGiven(buf, arg, types, nArgsMax);
    int k = findCmd(buf, types);
    if (k >= 0)
      invokeCmd(k, arg);
    else
    {
      usage();
    }
  }
  return 0;
}

// Background
void *cmBg(void *cmdline)
{
  char *buf = (char *)cmdline;
  // if (buf[0] == 0) {
  // }
  if (buf[0] == '!') // begins with !, execute it as
    system(buf + 1); // a normal shell cmd
  else
  {
    setArgsGiven(buf, arg, types, nArgsMax);
    int k = findCmd(buf, types);
    if (k >= 0)
    {
    }
    else
      invokeCmd(k, arg);
  }
  return (0);
}

void *systemBG(void *cmd)
{
  system((char *)cmd);
  return 0;
}

void ourgets(char *buf)
{
  fgets(buf, 1024, stdin);
  char *c = index(buf, '\n');
  if (c)
    *c = 0;
}

char *seperateSpaces(char *c)
{
  while (*c == ' ')
  {
    c++;
  }
  return c;
}

int main()
{
  char buf[1024]; // better not type longer than 1023 chars
  char *c;
  char *redir;
  char *token1;
  char *token2;
  char *token3;
  int bg, fd, out, tmp;

  usage();
  for (;;)
  {
    redir = 0; // clears out the input
    bg = 0;
    *buf = 0;               // clear old input
    printf("%s", "sh33% "); // prompt
    ourgets(buf);
    printf("cmd [%s]\n", buf); // just print out what we got as-is
    if (buf[0] == 0)
      continue;
    if (buf[0] == '#')
      continue; // this is a comment line, do nothing

    // Background
    c = strchr(buf, '&'); // searchs the char argument for the first occurence of &
    if (c)
    {
      bg = 1;
      *c = '\0'; // sets the pointer to null
    }
    // Redirection
    c = strchr(buf, '>');
    if (c)
    {
      redir = seperateSpaces(c + 1);        // sets redir to the next charcter
      *c = '\0';                            // sets the pointer to null
      fd = creat(redir, S_IRUSR | S_IWUSR); // Creates a file with permissions
      out = dup(1);
      close(1);
      dup2(fd, 1);
    }

    // Pipe
    c = strchr(buf, '|');
    if (c)
    {
      int numPipes = 0;
      int i = 0;
      while (buf[i] != '\0')
      {
        if (buf[i] == '|')
          numPipes++;
        i++;
      }

      printf("%d\n", numPipes);
      int pid, filedes[4];
      pipe(filedes);
      //*c = '\0';
      // c++;
      // pid = fork();
      token1 = strtok(buf, "|");
      token2 = strtok(NULL, "|");
      // *c = '\0';
      // c++;
      pid = fork();
      if (numPipes == 1)
      {
        if (pid < 0)
        {
          printf("Error\n");
        }
        if (pid == 0)
        {
          close(filedes[1]);
          tmp = dup(0); // Restores the STDIN file descriptor
          dup2(filedes[0], 0);
          cmdline(seperateSpaces(token2));
          close(filedes[0]);
          close(0);
          dup2(tmp, 0);
          close(tmp);
          raise(SIGTERM);
        }
        else
        {
          close(filedes[0]);
          tmp = dup(1); // Restores the STDOUT file descriptor
          dup2(filedes[1], 1);
          cmdline(token1);
          close(filedes[1]);
          close(1);
          dup2(tmp, 1);
          close(tmp);
          wait(NULL);
        }

      } // end of one pipe
      else if (numPipes == 2)
      {
        //  token1 = strtok(buf, "|");
        //  token2 = strtok(NULL, " |");
        token3 = strtok(NULL, "|");
        pipe(filedes + 2);
        // int pid2 = fork();
        if (pid < 0)
        {
          printf("Error\n");
        }
        else if (pid == 0)
        {

          int pid2 = fork();

          if (pid2 == 0)
          {
            // close(filedes[2]);
            close(filedes[0]);
            close(filedes[1]);
            close(filedes[3]);

            tmp = dup(0); // Restores the STDIN file descriptor
                          // work here
            dup2(filedes[2], 0);
            cmdline(seperateSpaces(token3));
            close(filedes[2]);
            close(0);
            dup2(tmp, 0);
            close(tmp);
            exit(0);
          }
          else
          {

            close(filedes[2]);
            close(filedes[1]);
            tmp = dup(0); // Restores the STDIN file descriptor
            dup2(filedes[0], 0);
            dup2(filedes[3], 1);
            // work here
            cmdline(seperateSpaces(token2));
            close(filedes[0]);
            close(filedes[3]);
            close(0);
            dup2(tmp, 0);
            close(tmp);

            exit(0);
          }
        }
        else
        {
          // Parent
          close(filedes[0]);
          close(filedes[3]);
          close(filedes[2]);
          tmp = dup(1); // Restores the STDOUT file descriptor
          dup2(filedes[1], 1);
          cmdline(token1);
          close(filedes[1]);
          close(1);
          dup2(tmp, 1);
          close(tmp);
          wait(NULL);
        }
      }
      continue;
    }

    if (buf[0] == '!')
    { // begins with !, execute it as
      if (bg)
      {
        pthread_t threadhandle;
        pthread_create(&threadhandle, 0, systemBG, buf + 1);
      }
      else
        system(buf + 1);
    } // a normal shell cmd
    else
    {
      setArgsGiven(buf, arg, types, nArgsMax);
      int k = findCmd(buf, types);
      char *copybuf = buf;
      if (k >= 0)
      {
        if (bg)
        {
          pthread_t threadhandle;
          pthread_create(&threadhandle, 0, cmBg, (void *)copybuf);
        }
        else
          invokeCmd(k, arg);
      }
      else
        usage();

      if (redir)
      {
        close(fd);
        dup2(out, 1);
        close(out);
        redir = 0;
      }
    }
  }
}

// -eof-