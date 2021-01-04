#include <faasm/faasm.h>

#include <cstring>
#include <string>

int main(int argc, char* argv[])
{
    char dest[50];

    strcpy(dest, "alpha ");

    ::strcat(dest, "beta gamma");

    const char* expected = "alpha beta gamma";

    if(strcmp(dest, expected) != 0) {
        printf("\"%s\" != \"%s\"\n", dest, expected);
        return 1;
    }

    return 0;
}
