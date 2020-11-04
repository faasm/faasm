#include "faasm/faasm.h"
#include "faasm/input.h"

#include <stdio.h>
#include <string.h>

FAASM_MAIN_FUNC()
{
    const char* inputStr = faasm::getStringInput("");

    const char* expected = "http://www.foobar.com";
    int retVal;
    if (strcmp(inputStr, expected) != 0) {
        printf("Expected %s but got %s\n", expected, inputStr);
        retVal = 1;
    } else {
        printf("Expected %s and got %s\n", expected, inputStr);
        retVal = 0;
    }

    return retVal;
}
