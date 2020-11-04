#include <stdio.h>

int main(int argc, char** argv)
{
    printf("Got %i arguments.\n", argc);

    for (int i = 0; i < argc; ++i) {
        printf("%i = %s\n", i, argv[i]);
    }

    return 0;
}
