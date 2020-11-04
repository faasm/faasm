#ifndef FAASM_FILES_H
#define FAASM_FILES_H

namespace faasm {
long getFileLength(const char* path);

char* readFileToString(const char* path);
}

#endif
