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

void doPwd(Arg *a)
{
  Directory *curDir = wd;
  Directory *parentDir;
  uint parentINode = 0;
  std::vector<std::string> pathVec;
  std::string path = "";

  while (parentINode != 1)
  {
    parentINode = curDir->iNumberOf((byte *)"..");
    parentDir = new Directory(fv, parentINode, 0);
    pathVec.push_back((char *)parentDir->nameOf(curDir->nInode));
    if (curDir != wd)
    {
      delete (curDir);
    }
    curDir = parentDir;
  }
  delete (parentDir);

  for (int i = pathVec.size() - 1; i >= 0; i--)
  {
    path = path + "/" + pathVec[i];
  }

  // std::cout << path << std::endl;
}

std::string getAbsPath(Directory *curDir)
{
  Directory *parentDir;
  uint parentINode = 0;
  std::vector<std::string> pathVec;
  std::string path = "";

  while (parentINode != 1)
  {
    parentINode = curDir->iNumberOf((byte *)"..");
    parentDir = new Directory(fv, parentINode, 0);
    pathVec.push_back((char *)parentDir->nameOf(curDir->nInode));
    if (curDir != wd)
    {
      delete (curDir);
    }
    curDir = parentDir;
  }
  delete (parentDir);

  for (int i = pathVec.size() - 1; i >= 0; i--)
  {
    path = path + "/" + pathVec[i];
  }

  return path;
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

void doLsLink(Arg * a)
{
  Directory* curDir;
  uint iNode = wd->iNumberOf((byte *) a[0].s);
  if (iNode != 0 && wd->fv->inodes.getType(iNode) == iTypeDirectory) {
    curDir = wd;
    wd = new Directory(fv, iNode, 0);
    doLsLong(a);
    delete(wd);
    wd = curDir;
  }
  else if (iNode != 0 && wd->fv->inodes.getType(iNode) == iTypeOrdinary) {
    printf("%7d %crw-rw-rw-    1 yourName yourGroup %7d Jul 15 12:34 %s\n",
	     iNode, '-', wd->fv->inodes.getFileSize(iNode), a[0].s);
  }
  else if (iNode != 0 && wd->fv->inodes.getType(iNode) == iTypeSoftLink) {
    uint bn = fv->inodes.getBlockNumber(iNode, 0);
    byte *blockData = new byte[fv->superBlock.nBytesPerBlock];
    fv->readBlock(bn, blockData);
    if (blockData[1] == '.') {
      blockData+=2;
    }
    printf("%7d %crw-rw-rw-    1 yourName yourGroup %7d Jul 15 12:34 %s -> %s\n",
	     iNode, 'l', wd->fv->inodes.getFileSize(iNode), a[0].s, (char *) blockData);
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
  printf("Incorrect flags for recursive ls");
}

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

std::vector<std::string> doMvPath(char *path, bool &invalidPath, bool &IsFile, bool &exists);

uint getLinkType(std::string path)
{
  Directory *startDir = new Directory(fv, wd->nInode, 0);
  char charPath[path.length() + 1];
  strcpy(charPath, path.c_str());
  bool tmp;
  std::vector<std::string> pathVec = doMvPath(charPath, tmp, tmp, tmp);
  Directory *pathDir = wd;
  wd = startDir;
  const char *lastEntry = pathVec[pathVec.size() - 1].c_str();
  uint in = pathDir->iNumberOf((byte *)lastEntry);
  if (pathDir->fv->inodes.getType(in) == iTypeOrdinary)
  {
    return iTypeOrdinary;
  }
  else if (pathDir->fv->inodes.getType(in) == iTypeDirectory)
  {
    return iTypeDirectory;
  }
  else if (pathDir->fv->inodes.getType(in) == iTypeSoftLink)
  {
    uint bn = fv->inodes.getBlockNumber(in, 0);
    byte *blockData = new byte[fv->superBlock.nBytesPerBlock];
    fv->readBlock(bn, blockData);
    if (blockData[1] == '.')
    {
      blockData += 2;
    }
    std::string pathStr = (char *)blockData;
    return getLinkType(pathStr);
  }
  else
    return 0;
}

bool successiveCD(char *path)
{
  bool rootDir = false;
  bool afterRoot = true;
  Directory *startDir = new Directory(fv, wd->nInode, 0);
  if (path[0] == '/')
  {
    rootDir = true;
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
          rootDir = false;
          return false;
        }
        else
        {
          rootDir = true;
        }
      }
    }
  }
  if (rootDir)
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
  if (afterRoot)
  {
    std::vector<std::string> pathVec = getPathVec(path);
    uint iNode = 0;
    const char *pathEntry;
    for (long unsigned int i = 0; i < pathVec.size(); i++)
    {
      pathEntry = pathVec[i].c_str(); 
      iNode = wd->iNumberOf((byte *)pathEntry);
      if (iNode != 0 && wd->fv->inodes.getType(iNode) == iTypeDirectory)
      {
        Directory *tmp = wd;
        wd = new Directory(fv, iNode, 0);
        if (tmp != startDir)
        {
          delete (tmp);
        }
      }
      else if (iNode != 0 && wd->fv->inodes.getType(iNode) == iTypeSoftLink)
      {
        uint bn = fv->inodes.getBlockNumber(iNode, 0);
        byte *blockData = new byte[fv->superBlock.nBytesPerBlock];
        fv->readBlock(bn, blockData);
        if (blockData[1] == '.')
        {
          blockData += 2;
        }
        std::string pathStr = (char *)blockData;
        uint type = getLinkType(pathStr);
        if (type == iTypeDirectory)
        {
          successiveCD((char *)blockData);
        }
        else
        {
          return false;
        }
      }
      else
      {
        if (wd != startDir)
        {
          delete (wd);
          wd = startDir;
        }
        return false;
      }
    }
    if (pathVec.size() == 0)
    {
      if (wd != startDir)
      {
        delete (wd);
        wd = startDir;
      }
      return false;
    }
  }
  if (wd != startDir)
  {
    delete (startDir);
  }
  return true;
}

std::vector<std::string> doMvPath(char *path, bool &invalidPath, bool &IsFile, bool &exists)
{
  bool rootDir = false;
  bool afterRoot = true;
  Directory *startDir = new Directory(fv, wd->nInode, 0);
  if (path[0] == '/')
  {
    rootDir = true;
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
          rootDir = false;
          break;
        }
        else
        {
          rootDir = true;
        }
      }
    }
  }
  if (rootDir)
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
      else if (iNode != 0 && wd->fv->inodes.getType(iNode) == iTypeSoftLink)
      {
        if (i != pathVec.size() - 1)
        {
          uint bn = fv->inodes.getBlockNumber(iNode, 0);
          byte *blockData = new byte[fv->superBlock.nBytesPerBlock];
          fv->readBlock(bn, blockData);
          if (blockData[1] == '.')
          {
            blockData += 2;
          }
          std::string pathStr = (char *)blockData;
          uint type = getLinkType(pathStr);
          if (type == iTypeDirectory)
          {
            bool success = successiveCD((char *)blockData);
            if (!success)
            {
              invalidPath = true;
              break;
            }
          }
          else
          {
            invalidPath = true;
            break;
          }
        }
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

void doRm(Arg * a)
{
  Directory * startDir = new Directory(fv, wd->nInode, 0);
  bool tmp;

  std::vector<std::string> pathVec = doMvPath(a[0].s, tmp, tmp, tmp);
  Directory* parentDir = wd;
  wd = startDir;

  const char* deleteEntity = pathVec[pathVec.size() - 1].c_str();

  uint in = parentDir->iNumberOf((byte *) deleteEntity);

  if (in == 0) {
    printf("File/Directory not found.\n");
    delete(parentDir);
    return;
  }
  uint numContents = 0;
  if (parentDir->fv->inodes.getType(in) == iTypeDirectory) {
    numContents = wd->lsInt(in);
  }
  if (parentDir->fv->inodes.getType(in) == iTypeDirectory && numContents == 0) {
    in = parentDir->deleteFile((byte *) deleteEntity, 1);
    printf("Successfully removed directory '%s' with inode %d and %d entries.\n", deleteEntity, in, numContents);
  }
  else if (parentDir->fv->inodes.getType(in) == iTypeDirectory && numContents != 0) {
    printf("Unable to remove directory '%s' with inode %d and %d entries.\n", deleteEntity, in, numContents);
  }
  else if (parentDir->fv->inodes.getType(in) == iTypeOrdinary) {
    int numLinks = fv->inodes.getLinkCount(in);
    int flags;
    if (numLinks == 0) {
      flags = 1;
    }
    else if (numLinks > 0) {
      flags = 0;
    }
    in = parentDir->deleteFile((byte *) deleteEntity, flags);
    fv->inodes.incLinkCount(in, -1);
    printf("Successfully removed file '%s' with inode %d.\n", deleteEntity, in);
  }
  else if (parentDir->fv->inodes.getType(in) == iTypeSoftLink) {
    int numLinks = fv->inodes.getLinkCount(in);
    int flags;
    if (numLinks == 0) {
      flags = 1;
    }
    else if (numLinks > 0) {
      flags = 0;
    }
    in = parentDir->deleteFile((byte *) deleteEntity, flags);
    fv->inodes.incLinkCount(in, -1);
    printf("Successfully removed symbolic link '%s' with inode %d.\n", deleteEntity, in);
  }
  delete(parentDir);
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
  printf("Incorrect flags for recursive rm.\n");
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
  uint in = wd->iNumberOf((byte *)a[0].s);
  if (in != 0)
  {
    printf("%s already exists.\n", a[0].s);
    return;
  }
  in = wd->createFile((byte *)a[0].s, 1);
  printf("inode: %d\n", in);
}

void doTouch(Arg *a)
{
  uint in = wd->iNumberOf((byte *)a[0].s);
  if (in != 0)
  {
    printf("'%s' already exists.\n", a[0].s);
    return;
  }
  in = wd->createFile((byte *)a[0].s, 0);
  printf("inode: %d\n", in);
}


void doChDir(Arg * a)
{
  bool rootDir;
  bool afterRoot = true;
  Directory* startDir = new Directory(fv, wd->nInode, 0);
  if (a[0].s[0] == '/') {
    rootDir = true;
    if (a[0].s[1] == 0) {
      afterRoot = false;
    }
    else if (a[0].s[1] == '/') {
      afterRoot = false;
      for (long unsigned int i = 0; i < strlen(a[0].s); i++) {
        if (a[0].s[i] != '/') {
          rootDir = false;
          printf("Changing directory failed.\n");
          break;
        }
        else {
          rootDir = true;
        }
      }
    }
  }
  if (rootDir) {
    Directory* childDir = wd;
    uint rootINode = 0;
    while (rootINode != 1) {
      rootINode = childDir->iNumberOf((byte *) "..");
      wd = new Directory(fv, rootINode, 0);
      if (childDir != startDir) {
        delete(childDir);
      }
      childDir = wd;
    }
  }
  if (afterRoot) {
    std::vector<std::string> pathVec = getPathVec(a[0].s);
    uint iNode = 0;
    const char* pathEntry;
    for (long unsigned int i = 0; i < pathVec.size(); i++) {
      pathEntry = pathVec[i].c_str(); 
      iNode = wd->iNumberOf((byte *) pathEntry);
      if (iNode != 0 && wd->fv->inodes.getType(iNode) == iTypeDirectory) {
        Directory* tmp = wd;
        wd = new Directory(fv, iNode, 0);
        if (tmp != startDir) {
          delete(tmp);
        }
      }
      else if (iNode != 0 && wd->fv->inodes.getType(iNode) == iTypeSoftLink) {
        uint bn = fv->inodes.getBlockNumber(iNode, 0);
        byte *blockData = new byte[fv->superBlock.nBytesPerBlock];
        fv->readBlock(bn, blockData);
        if (blockData[1] == '.') {
          blockData+=2;
        }
        std::string pathStr = (char *) blockData;
        uint type = getLinkType(pathStr);
        if (type == iTypeDirectory) {
          bool success = successiveCD((char *) blockData);
          if (!success) {
            printf("Changing directory failed.\n");
            if (wd != startDir) {
              delete(wd);
              wd = startDir;
            }
          }
        }
        else {
          printf("Changing directory failed.\n");
          if (wd != startDir) {
            delete(wd);// here
            wd = startDir;
          }
        }
      }
      else {
        printf("Changing directory failed.\n");
        if (wd != startDir) {
          delete(wd);
          wd = startDir;
        }
        break;
      }
    }
    if (pathVec.size() == 0) {
      printf("Changing directory failed.\n");
      if (wd != startDir) {
        delete(wd);
        wd = startDir;
      }
    }
  }
  if (wd != startDir) {
    delete(startDir);
  }
  doPwd(a);
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

void doMv(Arg * a)
{
  Directory * startDir = new Directory(fv, wd->nInode, 0);
  bool sourceInvalidPath = false;
  bool sourceIsFile = false;
  bool sourceExists = true;
  bool destInvalidPath = false;
  bool destIsFile = false;
  bool destExists = true;

  if (a[0].s[0] == '.') {
    printf("Cannot move '.' or '..'.\n");
    return;
  }

  char* destPath = new char[strlen(a[1].s)+1]; // allocate for string and ending \0
  strcpy(destPath, a[1].s);

  std::vector<std::string> sourceVec = doMvPath(a[0].s, sourceInvalidPath, sourceIsFile, sourceExists);
  Directory* sourceDir = new Directory(fv, wd->nInode, 0);
  delete(wd);
  wd = new Directory(fv, startDir->nInode, 0);
  if (sourceVec.size() == 0) {
    delete(destPath);
    if (sourceDir != wd) {
      delete(sourceDir);
    }
    return;
  }
  std::vector<std::string> destVec = doMvPath(destPath, destInvalidPath, destIsFile, destExists);
  if (destVec.size() == 0) {
    destVec.push_back(".");
  }
  delete(destPath);
  Directory* destDir = new Directory(fv, wd->nInode, 0);
  delete(wd);
  wd = new Directory(fv, startDir->nInode, 0);

  if (!sourceExists || sourceInvalidPath || destInvalidPath || (destExists && destIsFile)) {
    if (sourceDir != wd) {
      delete(sourceDir);
    }
    if (destDir != wd) {
      delete(destDir);
    }
    printf("This cmd did not work\n");
    return;
  }
  else if (!destExists) {
    uint flags;
    const char* sourceFile = sourceVec[sourceVec.size() - 1].c_str();
    const char* destName = destVec[destVec.size() - 1].c_str();
    uint iNode = sourceDir->iNumberOf((byte *) sourceFile);
    uint sourceType = sourceDir->fv->inodes.getType(iNode);
    if (sourceIsFile || sourceType == iTypeSoftLink) {
      flags = 0;
    }
    else {
      flags = 1;
      if (fileInPath(destVec, iNode)) {
        printf("Cannot move a directory into its own subdirectory.\n");
        return;
      }
    }
    sourceDir->deleteFile((byte *) sourceFile, 0);
    destDir->customCreateFile((byte *) destName, iNode, flags);
    if (sourceType == iTypeSoftLink) {
      destDir->fv->inodes.setType(iNode, iTypeSoftLink);
    }
    if (flags == 1) {
      Directory* newDir = new Directory(fv, iNode, 0);
      newDir->customDeleteFile((byte *) "..", 0);
      uint destINode = destDir->iNumberOf((byte *) ".");
      newDir->customCreateFile((byte *) "..", destINode, flags);
      if (newDir != wd) {
        delete(newDir);
      }
    }
    printf("Renamed successfully.\n");
  }
  else if (destExists) {
    uint flags;
    const char* sourceFile = sourceVec[sourceVec.size() - 1].c_str();
    const char* destName = destVec[destVec.size() - 1].c_str();
    uint iNode = sourceDir->iNumberOf((byte *) sourceFile);
    uint destINode = destDir->iNumberOf((byte *) destName);
    if (destDir->fv->inodes.getType(destINode) == iTypeDirectory) {
      Directory* tmp = destDir;
      destDir = new Directory(fv, destINode, 0);
      if (tmp != wd) {
        delete(tmp);
      }
      delete(startDir);
    }
    else if (destDir->fv->inodes.getType(destINode) == iTypeSoftLink) {
      uint bn = fv->inodes.getBlockNumber(destINode, 0);
      byte *blockData = new byte[fv->superBlock.nBytesPerBlock];
      fv->readBlock(bn, blockData);
      if (blockData[1] == '.') {
        blockData+=2;
      }
      bool success = successiveCD((char*) blockData);
      destDir = wd;
      wd = startDir;
      if (!success) {
        printf("Move did not work.\n");
        return;
      }
    }
    if (destDir->iNumberOf((byte *) sourceFile) != 0) {
      if (destDir != wd) {
        delete(destDir);
      }
      printf("Directory already exists.\n");
      return;  
    }
    uint sourceType = sourceDir->fv->inodes.getType(iNode);
    if (sourceIsFile || sourceType == iTypeSoftLink) {
      flags = 0;
    }
    else {
      flags = 1;
      if (fileInPath(destVec, iNode)) {
        if (destDir != wd) {
          delete(destDir);
        }
        printf("Cannot move a directory into its own subdirectory.\n");
        return;
      }
    }
    sourceDir->deleteFile((byte *) sourceFile, 0);
    destDir->customCreateFile((byte *) sourceFile, iNode, flags);
    if (sourceType == iTypeSoftLink) {
      destDir->fv->inodes.setType(iNode, iTypeSoftLink);
    }
    if (flags == 1) {
      Directory* newDir = new Directory(fv, iNode, 0);
      newDir->customDeleteFile((byte *) "..", 0);
      newDir->customCreateFile((byte *) "..", destINode, flags);
      if (newDir != wd) {
        delete(newDir);
      }
    }
    printf("Moved successfully.\n");
  }

  if (sourceDir != wd) {
    delete(sourceDir);
  }
  if (destDir != wd) {
    delete(destDir);
  }
}
void doHardLink(Arg *a)
{
  Directory *startDir = new Directory(fv, wd->nInode, 0);
  bool sourceInvalidPath = false;
  bool sourceIsFile = false;
  bool sourceExists = true;
  bool destInvalidPath = false;
  bool destIsFile = false;
  bool destExists = true;

  char *destPath = new char[strlen(a[1].s) + 1]; // allocate for string and ending \0
  strcpy(destPath, a[1].s);

  std::vector<std::string> sourceVec = doMvPath(a[0].s, sourceInvalidPath, sourceIsFile, sourceExists);
  Directory *sourceDir = new Directory(fv, wd->nInode, 0);
  delete (wd);
  wd = new Directory(fv, startDir->nInode, 0);

  std::vector<std::string> destVec = doMvPath(destPath, destInvalidPath, destIsFile, destExists);
  if (destVec.size() == 0)
  {
    destVec.push_back(".");
  }
  delete (destPath);
  Directory *destDir = new Directory(fv, wd->nInode, 0);
  delete (wd);
  wd = new Directory(fv, startDir->nInode, 0);

  const char *sourceFile = sourceVec[sourceVec.size() - 1].c_str();
  uint iNode = sourceDir->iNumberOf((byte *)sourceFile);
  const char *destName = destVec[destVec.size() - 1].c_str();
  uint destINode = destDir->iNumberOf((byte *)destName);

  if (!sourceExists || sourceInvalidPath || sourceDir->fv->inodes.getType(iNode) == iTypeDirectory || destInvalidPath || (destExists && destIsFile))
  {
    if (sourceDir != wd)
    {
      delete (sourceDir);
    }
    if (destDir != wd)
    {
      delete (destDir);
    }
    printf("Creation of hard link failed.\n");
    return;
  }
  else if (destExists && !destIsFile)
  {
    if (destDir->fv->inodes.getType(destINode) == iTypeDirectory)
    {
      Directory *tmp = destDir;
      destDir = new Directory(fv, destINode, 0);
      if (tmp != wd)
      {
        delete (tmp);
      }
    }
    else if (destDir->fv->inodes.getType(destINode) == iTypeSoftLink)
    {
      uint bn = fv->inodes.getBlockNumber(destINode, 0);
      byte *blockData = new byte[fv->superBlock.nBytesPerBlock];
      fv->readBlock(bn, blockData);
      if (blockData[1] == '.')
      {
        blockData += 2;
      }
      std::string pathStr = (char *)blockData;
      uint destType = getLinkType(pathStr);
      if (destType == iTypeDirectory)
      {
        bool success = successiveCD((char *)blockData);
        destDir = wd;
        wd = startDir;
        if (!success)
        {
          printf("Creation of hard link failed.\n");
          return;
        }
      }
      else
      {
        wd = startDir;
        printf("Creation of hard link failed.\n");
        return;
      }
    }
    if (destDir->iNumberOf((byte *)sourceFile) == 0)
    {
      uint type = sourceDir->fv->inodes.getType(iNode);
      uint newINode = destDir->customCreateFile((byte *)sourceFile, iNode, 0);
      if (type == iTypeSoftLink)
      {
        destDir->fv->inodes.setType(newINode, iTypeSoftLink);
      }
      printf("Hard link with inode %d created successfully.\n", newINode);
      fv->inodes.incLinkCount(iNode, 1);
    }
    else
    {
      printf("File with name %s already exists in the destination directory.\n", sourceFile);
    }
  }
  else if (!destExists)
  {
    uint type = sourceDir->fv->inodes.getType(iNode);
    uint newINode = destDir->customCreateFile((byte *)destName, iNode, 0);
    if (type == iTypeSoftLink)
    {
      destDir->fv->inodes.setType(newINode, iTypeSoftLink);
    }
    printf("Hard link with inode %d created successfully.\n", newINode);
  }
  else
  {
    printf("Creation of hard link failed.\n");
  }

  if (sourceDir != wd)
  {
    delete (sourceDir);
  }
  if (destDir != wd)
  {
    delete (destDir);
  }
  wd = startDir;
}

void doHardLinkCurDir(Arg *a)
{
  
  a[1].s = ".";
  doHardLink(a);
}

void doSoftLink(Arg *a)
{
  if (strcmp(a[0].s, "-s") != 0)
  {
    printf("Incorrect flags for symbolic link creation.\n");
  }
  Directory *startDir = new Directory(fv, wd->nInode, 0);
  bool sourceInvalidPath = false;
  bool sourceIsFile = false;
  bool sourceExists = true;
  bool destInvalidPath = false;
  bool destIsFile = false;
  bool destExists = true;

  char *destPath = new char[strlen(a[2].s) + 1]; // allocate for string and ending \0
  strcpy(destPath, a[2].s);
  // end citation

  std::vector<std::string> sourceVec = doMvPath(a[1].s, sourceInvalidPath, sourceIsFile, sourceExists);
  Directory *sourceDir = new Directory(fv, wd->nInode, 0);
  delete (wd);
  wd = new Directory(fv, startDir->nInode, 0);

  std::vector<std::string> destVec = doMvPath(destPath, destInvalidPath, destIsFile, destExists);
  if (destVec.size() == 0)
  {
    destVec.push_back(".");
  }
  delete (destPath);
  Directory *destDir = new Directory(fv, wd->nInode, 0);
  delete (wd);
  wd = new Directory(fv, startDir->nInode, 0);

  const char *sourceFile = sourceVec[sourceVec.size() - 1].c_str();
  const char *destName = destVec[destVec.size() - 1].c_str();
  uint destINode = destDir->iNumberOf((byte *)destName);

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
    printf("Creation of symbolic link failed.\n");
    return;
  }
  else if (destExists && !destIsFile)
  {
    if (destDir->fv->inodes.getType(destINode) == iTypeDirectory)
    {
      Directory *tmp = destDir;
      destDir = new Directory(fv, destINode, 0);
      if (tmp != wd)
      {
        delete (tmp);
      }
    }
    else if (destDir->fv->inodes.getType(destINode) == iTypeSoftLink)
    {
      uint bn = fv->inodes.getBlockNumber(destINode, 0);
      byte *blockData = new byte[fv->superBlock.nBytesPerBlock];
      fv->readBlock(bn, blockData);
      if (blockData[1] == '.')
      {
        blockData += 2;
      }
      std::string pathStr = (char *)blockData;
      uint destType = getLinkType(pathStr);
      if (destType == iTypeDirectory)
      {
        bool success = successiveCD((char *)blockData);
        destDir = wd;
        wd = startDir;
        if (!success)
        {
          printf("Creation of symbolic link failed.\n");
          return;
        }
      }
      else
      {
        wd = startDir;
        printf("Creation of symbolic link failed.\n");
        return;
      }
    }
    if (destDir->iNumberOf((byte *)sourceFile) == 0)
    {
      std::string sourceAbsPath = getAbsPath(sourceDir);
      sourceAbsPath = sourceAbsPath + "/" + sourceVec[sourceVec.size() - 1];
      const char *sourcePathPointer = sourceAbsPath.c_str();
      uint linkINode = destDir->createFile((byte *)sourceFile, 0);
      destDir->fv->inodes.setType(linkINode, iTypeSoftLink);
      File *pathFile = new File(fv, linkINode);
      sourceDir = new Directory(fv, sourceDir->nInode, 0);
      pathFile->appendBytes((byte *)sourcePathPointer, strlen(sourcePathPointer) + strlen(sourceFile));
      delete pathFile;
      printf("Symbolic link with inode %d created successfully.\n", linkINode);
    }
    else
    {
      printf("File or directory with name %s already exists in the destination directory.\n", sourceFile);
    }
  }
  else if (!destExists)
  {
    std::string sourceAbsPath = getAbsPath(sourceDir);
    sourceAbsPath = sourceAbsPath + "/" + sourceVec[sourceVec.size() - 1];
    const char *sourcePathPointer = sourceAbsPath.c_str();
    uint linkINode = destDir->createFile((byte *)destName, 0);
    fv->inodes.setType(linkINode, iTypeSoftLink);
    File *pathFile = new File(fv, linkINode);
    sourceDir = new Directory(fv, sourceDir->nInode, 0);
    pathFile->appendOneBlock((byte *)sourcePathPointer, strlen(sourcePathPointer) + strlen(destName)); // only has . here
    delete pathFile;
    printf("Symbolic link with inode %d created successfully.\n", linkINode);
  }
  else
  {
    printf("Creation of symbolic link failed.\n");
  }

  if (sourceDir != wd && sourceDir != NULL)
  {
    delete (sourceDir);
  }
  if (destDir != wd && destDir != NULL)
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
    {"ls", "s", "v", doLsLink},
    {"ls", "ss", "v", lsRecursive},
    {"lslong", "", "v", doLsLong},
    {"mkdir", "s", "v", doMkDir},
    {"touch", "s", "v", doTouch},
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
    {"wrdisk", "sus", "", doWriteDisk},
    {"ln", "ss", "v", doHardLink},
    {"ln", "s", "v", doHardLinkCurDir},
    {"ln", "sss", "v", doSoftLink}};

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