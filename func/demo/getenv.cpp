#include "faasm/faasm.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

FAASM_MAIN_FUNC() {
    // Check predefined var
    const char *expectedCtype = "en_GB.UTF-8";
    char *preset = getenv("LC_CTYPE");
    if (strcmp(preset, expectedCtype) == 0) {
        printf("LC_CTYPE as expected\n");
    } else {
        printf("LC_CTYPE not as expected (got %s, expected %s)\n", preset, expectedCtype);
        return 1;
    }

    const char *varName = "FOOBAR";
    char *unset = getenv(varName);

    if (unset != nullptr) {
        printf("Was expecting %s not to be set \n", varName);
        return 1;
    }

    const char *expected = "BAZ";
    setenv("FOOBAR", expected, 0);

    char *actual = getenv(varName);
    printf("FOOBAR=%s\n", actual);

    if (strcmp(actual, "BAZ") != 0) {
        printf("Was expecting %s to be set to %s\n", varName, expected);
        return 1;
    }

    return 0;
}
