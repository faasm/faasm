#include <sys/stat.h>

int main(int argc, char* argv[])
{
    struct stat64 s
    {};
    stat64("/", &s);

    return 0;
}
