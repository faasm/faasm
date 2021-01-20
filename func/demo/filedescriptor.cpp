#include "faasm/faasm.h"

#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <string>

#include <unistd.h>

std::string contentA = "This is my file.\n\
There are many like it, but this one is mine.\n\
My file is my best friend. It is my life.\n";

std::string contentB = "I must master it as I must master my life.\n\
Without me, my file is useless.\n\
Without my file, I am useless.\n";

void deleteFile(const char* fileName)
{
    // Delete file if it exists
    if (access(fileName, F_OK) != -1) {
        int removeRes = remove(fileName);
        if (removeRes != 0) {
            printf("Failed to remove %s: %i (%s)\n",
                   fileName,
                   errno,
                   strerror(errno));
            exit(1);
        };
    }
}

bool seekReadCheck(int fd, int offset, int whence, const std::string& expected)
{
    ::lseek(fd, offset, whence);

    char buf[expected.size()];

    ::read(fd, buf, expected.size());

    std::string actual(buf, buf + expected.size());

    if (actual != expected) {
        printf("String not as expected: %s != %s\n",
               actual.c_str(),
               expected.c_str());
        return false;
    }

    return true;
}

int main(int argc, char* argv[])
{
    const char* filePathA = "fileA.txt";

    deleteFile(filePathA);

    // Write initial content
    int fdA = open(filePathA, O_RDWR | O_CREAT);
    ::write(fdA, contentA.c_str(), contentA.size());

    // Dup the file descriptor and write more content
    int fdB = dup(fdA);
    ::write(fdB, contentB.c_str(), contentB.size());

    // Open new descriptors on the same file
    int fdC = ::open(filePathA, O_RDWR);
    int fdD = ::dup(fdA);

    // Prepare some expectations and offsets
    std::string expectedA = "I am useless";
    std::string expectedB = "There are many like it";
    std::string expectedC = "There are many";
    std::string expectedD = "There are many";

    int relativeOffsetA = -20;
    int absoluteOffsetB = 10;

    int relativeOffsetC = 35;
    int absoluteOffsetD1 = 30;
    int absoluteOffsetD2 = 10;

    bool success = true;
    success &= seekReadCheck(fdA, relativeOffsetA, SEEK_CUR, "foobar");
    success &= seekReadCheck(fdB, absoluteOffsetB, SEEK_SET, "baz bar foo");

    success &= seekReadCheck(fdC, relativeOffsetC, SEEK_CUR, "balh balh balh");

    success &= seekReadCheck(fdD, absoluteOffsetD1, SEEK_SET, "ffffffffffff");
    success &= seekReadCheck(fdD, absoluteOffsetD2, SEEK_SET, "gfggggsgsgsgs");

    if (success) {
        return 0;
    } else {
        return 1;
    }
}
