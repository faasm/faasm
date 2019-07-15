#include "faasm/faasm.h"

#include <stdio.h>
#include <sys/utsname.h>
#include <cstring>

/**
 * Checks uname gives info back
 */

int _check_value(const char* fieldName, const char* actual, const char* expected) {
    if(strcmp(actual, expected) != 0) {
        printf("uname field %s not as expected (actual=%s expected=%s)\n", fieldName, actual, expected);
        return 1;
    }

    return 0;
}

FAASM_MAIN_FUNC() {
    // Make the call
    utsname u{};
    uname(&u);

    // Check the result
    int res = 0;
    res += _check_value("sysname", u.sysname, "Linux");
    res += _check_value("nodename", u.nodename, "faasm");
    res += _check_value("release", u.release, "1.0.0");
    res += _check_value("version", u.version, "Faasm 123");
    res += _check_value("machine", u.machine, "x86");
    res += _check_value("domainname", u.domainname, "(none)");

    if(res > 0) {
        printf("ERROR - uname check failed\n");
        return 1;
    }

    return 0;
}
