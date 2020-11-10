#include "faasm/faasm.h"

#include <cstring>
#include <pwd.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

int main(int argc, char* argv[])
{
    uid_t uid = getuid();
    gid_t gid = getgid();
    printf("uid, gid: %u, %u\n", uid, gid);

    passwd* pwdPtr = getpwuid(uid);
    printf("Name:     %s\n", pwdPtr->pw_name);
    printf("Password: %s\n", pwdPtr->pw_passwd);
    printf("uid:      %u\n", pwdPtr->pw_uid);
    printf("gid:      %u\n", pwdPtr->pw_gid);
    printf("Gecos:    %s\n", pwdPtr->pw_gecos);
    printf("Home:     %s\n", pwdPtr->pw_dir);
    printf("Shell:    %s\n", pwdPtr->pw_shell);

    if (strcmp(pwdPtr->pw_name, "faasm") != 0) {
        return 1;
    }

    if (pwdPtr->pw_uid != 1000) {
        return 1;
    }

    return 0;
}
