#include <stdio.h>
#include <string>
#include <string.h>


namespace faasm {
    int *splitStringIntoInts(const char *strIn, int nInts) {
        char *strCopy = new char[strlen(strIn)];
        strcpy(strCopy, strIn);

        char *nextSubstr = strtok(strCopy, " ");
        int *result = new int[nInts];

        int i = 0;
        while (nextSubstr != NULL) {
            result[i] = std::stoi(nextSubstr);
            nextSubstr = strtok(NULL, " ");
            i++;
        }

        delete[] strCopy;

        return result;
    }
}
