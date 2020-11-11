#include <stdio.h>
#include <string.h>

int main(int argc, char* argv[])
{
    char dest[] = "alpha beta";
    const char src[] = "gamma";

    memmove(dest, src, 5);
    if (strcmp(dest, "gamma beta") != 0) {
        printf("memmove not as expected (%s)\n", dest);
        return 1;
    } else {
        printf("memmove as expected\n");
        return 0;
    }
}
