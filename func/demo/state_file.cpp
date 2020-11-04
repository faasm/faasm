#include <faasm/faasm.h>
#include <stdio.h>
#include <string.h>

FAASM_MAIN_FUNC()
{
    const char* filePath = "/tmp/state_file_test";
    const char* key = "state_file_test";
    const char* dummyContent = "blah blah foo foo";
    size_t contentLen = strlen(dummyContent);

    // Write something to the file
    FILE* f = fopen(filePath, "w");
    if (f == NULL) {
        printf("Error opening file at %s\n", filePath);
        return 1;
    }

    fprintf(f, "%s", dummyContent);
    fclose(f);

    // Ask for the file to be written to state
    size_t nBytesWritten = faasmWriteStateFromFile(key, filePath);
    if (nBytesWritten != contentLen) {
        printf("Expected %li bytes to be written but got %li\n",
               contentLen,
               nBytesWritten);
        return 1;
    }

    // Read in the state
    auto buffer = new uint8_t[contentLen];
    faasmReadState(key, buffer, contentLen);
    char* actual = reinterpret_cast<char*>(buffer);

    if (strcmp(actual, dummyContent) != 0) {
        printf("State not as expected (expected %s but got %s)\n",
               dummyContent,
               actual);
        return 1;
    }

    return 0;
}
