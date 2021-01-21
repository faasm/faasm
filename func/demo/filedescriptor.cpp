#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <string>
#include <vector>

#include <unistd.h>

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

bool validatePermissions()
{
    std::string filePath = "foobar.txt";
    deleteFile(filePath.c_str());

    // Should fail, as file doesn't exist
    int roFd = open(filePath.c_str(), O_RDONLY);
    if (roFd > 0) {
        printf("Should not be able to open read-only fd\%s", strerror(errno));
        return false;
    }

    // Should also fail as file doesn't exist
    int woFd = open(filePath.c_str(), O_WRONLY);
    if (woFd > 0) {
        printf("Should not be able to open write-only fd\%s", strerror(errno));
        return false;
    }

    // Should also fail as file doesn't exist
    int rwFd = open(filePath.c_str(), O_RDWR);
    if (rwFd > 0) {
        printf("Should not be able to open read-write fd\%s", strerror(errno));
        return false;
    }

    // With create flag, should work
    int woFdCreate = open(filePath.c_str(), O_WRONLY | O_CREAT);
    if (woFdCreate < 0) {
        printf("Failed to open write-only/ create fd\%s", strerror(errno));
        return false;
    }

    // Delete the file again
    deleteFile(filePath.c_str());

    // Read-write with create should also work
    int rwFdCreate = open(filePath.c_str(), O_RDWR | O_CREAT);
    if (rwFdCreate < 0) {
        printf("Failed to open read-write/ create fd\%s", strerror(errno));
        return false;
    }

    return true;
}

bool seekReadCheck(int fd,
                   int offset,
                   int whence,
                   int expectedOffset,
                   const std::string& expected)
{
    off_t newOffset = ::lseek(fd, offset, whence);

    if (newOffset < 0) {
        printf("Error with seek: %s\n", strerror(errno));
        return false;
    }

    if (newOffset != expectedOffset) {
        printf("New offset not as expected (\"%s\"): %lli != %i\n",
               expected.c_str(),
               newOffset,
               expectedOffset);
        return false;
    }

    char buf[expected.size()];

    ssize_t bytesRead = ::read(fd, buf, expected.size());

    if (bytesRead != expected.size()) {
        printf("Failed reading \"%s\" (%li != %li): %s\n",
               expected.c_str(),
               bytesRead,
               expected.size(),
               strerror(errno));
        return false;
    }

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
    // Start off with the basic permissions
    validatePermissions();

    const char* filePath = "fileA.txt";
    deleteFile(filePath);

    std::string contentA = "This is my file.\n\
There are many like it, but this one is mine.\n\
My file is my best friend. It is my life.\n";

    std::string contentB = "I must master it as I must master my life.\n\
Without me, my file is useless.\n\
Without my file, I am useless.\n";

    // Write initial content
    int fdA = open(filePath, O_RDWR | O_CREAT);
    if (fdA < 0) {
        printf("Failed opening file: %s\n", strerror(errno));
        return 1;
    }

    ssize_t bytesWrittenA = ::write(fdA, contentA.c_str(), contentA.size());
    if (bytesWrittenA != contentA.size()) {
        printf("Failed writing content A (%li): %s\n",
               bytesWrittenA,
               strerror(errno));
        return 1;
    }

    // Dup the file descriptor and write more content
    int fdB = dup(fdA);
    if (fdB < 0) {
        printf("Failed duping file descriptor: %s\n", strerror(errno));
        return 1;
    }

    ssize_t bytesWrittenB = ::write(fdB, contentB.c_str(), contentB.size());
    if (bytesWrittenB != contentB.size()) {
        printf("Failed writing content B (%li): %s\n",
               bytesWrittenB,
               strerror(errno));
        return 1;
    };

    // Check the offsets
    off_t offA = ::lseek(fdA, 0, SEEK_CUR);
    off_t offB = ::lseek(fdB, 0, SEEK_CUR);

    if (offA < 0) {
        printf("Failed seek A: %s\n", strerror(errno));
        return 1;
    }

    if (offB < 0) {
        printf("Failed seek B: %s\n", strerror(errno));
        return 1;
    }

    if (offA != offB) {
        printf("Dup-ed fds with different offsets %lli != %lli\n", offA, offB);
        return 1;
    }

    size_t expectedOffset = contentA.size() + contentB.size();
    if (offA != (off_t)expectedOffset) {
        printf("Offset not as expected: %lli != %lu\n", offA, expectedOffset);
        return 1;
    }

    // Open new descriptors on the same file
    int fdC = ::open(filePath, O_RDONLY);
    if (fdC < 0) {
        printf("Failed opening second fd: %s\n", strerror(errno));
        return 1;
    }

    int fdD = ::dup(fdC);
    if (fdD < 0) {
        printf("Failed duping second fd: %s\n", strerror(errno));
        return 1;
    }

    off_t offC = ::lseek(fdC, 0, SEEK_CUR);
    off_t offD = ::lseek(fdD, 0, SEEK_CUR);

    if (offC < 0) {
        printf("Seek C failed: %s\n", strerror(errno));
        return 1;
    }

    if (offD < 0) {
        printf("Seek D failed: %s\n", strerror(errno));
        return 1;
    }

    if (offC != offD || offC != 0 || offD != 0) {
        printf(
          "Expected new fds to have offset zero: %lli and %lli\n", offC, offD);
    }

    // Prepare some expectations and offsets
    std::string expectedA = "I am useless";
    std::string expectedB = "There are many like it";
    std::string expectedC = "I must master it";
    std::string expectedD1 = "It is my life";
    std::string expectedD2 = "Without my file";

    int relativeOffsetA = -(expectedA.size() + 2);
    int absoluteOffsetB = 17;

    int relativeOffsetC = contentA.size();

    int relativeOffsetD1 = -31;
    int absoluteOffsetD2 = contentA.size() + contentB.size() - 31;

    if (!seekReadCheck(
          fdA, relativeOffsetA, SEEK_CUR, offA + relativeOffsetA, expectedA)) {
        return 1;
    }

    if (!seekReadCheck(
          fdB, absoluteOffsetB, SEEK_SET, absoluteOffsetB, expectedB)) {
        return 1;
    }

    if (!seekReadCheck(
          fdC, relativeOffsetC, SEEK_CUR, relativeOffsetC, expectedC)) {
        return 1;
    }

    // Note that this offset includes the reading done in the step before
    if (!seekReadCheck(fdD,
                       relativeOffsetD1,
                       SEEK_CUR,
                       relativeOffsetC + expectedC.size() + relativeOffsetD1,
                       expectedD1)) {
        return 1;
    }

    if (!seekReadCheck(
          fdD, absoluteOffsetD2, SEEK_SET, absoluteOffsetD2, expectedD2)) {
        return 1;
    }

    return 0;
}
