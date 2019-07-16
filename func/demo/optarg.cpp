/**
 * Function to check use of optarg still compiles
 */

#include <stdio.h>
#include <getopt.h>


int main(int argc, char *argv[]) {
    int option = -1;

    while ((option = getopt (argc, argv, "a:b:")) != -1)  {
        switch (option) {
            case 'a':
                printf("This should never be reached\n");
                break;
            case 'b':
                printf("This should never be reached\n");
                break;
            default:
                printf("This might be reached\n");
                break;
        }
    }

    return 0;
}
