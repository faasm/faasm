#include "faasm/faasm.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

bool checkPredefined(const char* name, const char* expected)
{
    char* actual = getenv(name);
    if (strcmp(actual, expected) == 0) {
        printf("%s as expected (%s)\n", name, expected);
        return false;
    } else {
        printf(
          "%s not as expected (got %s, expected %s)\n", name, actual, expected);
        return true;
    }
}

int main(int argc, char* argv[])
{
    // Check predefined var
    bool failed = false;
    failed |= checkPredefined("PYTHONNOUSERSITE", "on");
    failed |= checkPredefined("PYTHONHOME", "/");
    failed |= checkPredefined("PYTHON_PATH", "/");
    failed |= checkPredefined("PYTHONWASM", "1");

    if (failed) {
        return 1;
    }

    const char* varName = "FOOBAR";
    char* unset = getenv(varName);

    if (unset != nullptr) {
        printf("Was expecting %s not to be set \n", varName);
        return 1;
    }

    const char* expected = "BAZ";
    setenv("FOOBAR", expected, 0);

    char* actual = getenv(varName);
    printf("FOOBAR=%s\n", actual);

    if (strcmp(actual, "BAZ") != 0) {
        printf("Was expecting %s to be set to %s\n", varName, expected);
        return 1;
    }

    return 0;
}
