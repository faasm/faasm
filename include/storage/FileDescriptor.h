#pragma once

#include <dirent.h>
#include <string>
#include <unordered_map>
#include <vector>

// See wasi-libc/libc-bottom-half/cloudlibc/src/libc/fcntl/openat.c
#define WASI_RIGHTS_WRITE                                                      \
    (__WASI_RIGHT_FD_DATASYNC | __WASI_RIGHT_FD_WRITE |                        \
     __WASI_RIGHT_FD_ALLOCATE | __WASI_RIGHT_FD_FILESTAT_SET_SIZE)
#define WASI_RIGHTS_READ (__WASI_RIGHT_FD_READDIR | __WASI_RIGHT_FD_READ)

// The root fd comes after stdin, stdout and stderr
#define DEFAULT_ROOT_FD 4

namespace storage {
std::string prependRuntimeRoot(const std::string& originalPath);

enum OpenMode
{
    CREATE,
    DIRECTORY,
    EXCL,
    TRUNC,
    NONE,
};

enum ReadWriteType
{
    READ_ONLY,
    READ_WRITE,
    WRITE_ONLY,
    CUSTOM,
};

OpenMode getOpenMode(uint16_t openFlags);

ReadWriteType getRwType(uint64_t rights);

class DirEnt
{
  public:
    uint64_t next;
    uint8_t type;
    uint64_t ino;
    std::string path;
};

class Stat
{
  public:
    bool failed;
    uint16_t wasiErrno;

    uint8_t wasiFiletype;

    uint64_t st_dev;
    uint64_t st_ino;
    uint64_t st_nlink;
    uint64_t st_size;
    uint32_t st_mode;
    uint64_t st_atim;
    uint64_t st_mtim;
    uint64_t st_ctim;
};

class FileDescriptor
{
  public:
    static FileDescriptor stdinFactory();

    static FileDescriptor stdoutFactory();

    static FileDescriptor stderrFactory();

    FileDescriptor() = default;

    DirEnt iterNext();

    bool iterStarted();

    bool iterFinished();

    void iterBack();

    void iterReset();

    size_t copyDirentsToWasiBuffer(uint8_t* buffer, size_t bufferLen);

    Stat stat(const std::string& relativePath = "");

    bool unlink(const std::string& relativePath = "");

    bool rmdir(const std::string& relativePath = "");

    bool rename(const std::string& newPath,
                const std::string& relativePath = "");

    ssize_t readLink(const std::string& relativePath,
                     char* buffer,
                     size_t bufferLen);

    bool pathOpen(uint32_t lookupFlags, uint32_t openFlags, int32_t fdFlags);

    bool updateFlags(int32_t fdFlags);

    void close();

    bool mkdir(const std::string& dirPath);

    uint16_t seek(uint64_t offset, int wasiWhence, uint64_t* newOffset);

    uint64_t tell();

    uint8_t wasiPreopenType;

    int getLinuxFd();

    int getLinuxErrno();

    uint16_t getWasiErrno();

    uint64_t getActualRightsBase();

    uint64_t getActualRightsInheriting();

    void setActualRights(uint64_t rights, uint64_t inheriting);

    std::string absPath(const std::string& relativePath);

    void setPath(const std::string& newPath);

    std::string getPath();

  private:
    static FileDescriptor stdFdFactory(int stdFd, const std::string& devPath);

    void loadDirContents();

    std::string path;

    bool rightsSet = false;
    uint64_t actualRightsBase;
    uint64_t actualRightsInheriting;

    int linuxFd = -1;
    int linuxMode = -1;
    int linuxFlags = -1;
    int linuxErrno = 0;

    uint16_t wasiErrno = 0;

    bool dirContentsLoaded = false;
    std::vector<DirEnt> dirContents;
    int dirContentsIdx = 0;
};
}
