#include "faasm/faasm.h"

#include <stdio.h>
#include <sys/types.h>
#include <pwd.h>
#include <unistd.h>

FAASM_MAIN_FUNC() {
    uid_t uid = getuid();
    gid_t gid = getgid();
    printf("uid, gid: %u, %u\n", uid, gid);

    passwd *pwdPtr = getpwuid(uid);
    printf("Name:     %s\n", pwdPtr->pw_name);
    printf("Password: %s\n", pwdPtr->pw_passwd);
    printf("uid:      %u\n", pwdPtr->pw_uid);
    printf("gid:      %u\n", pwdPtr->pw_gid);
    printf("Gecos:    %s\n", pwdPtr->pw_gecos);
    printf("Home:     %s\n", pwdPtr->pw_dir);
    printf("Shell:    %s\n", pwdPtr->pw_shell);

    return 0;
}
