#include "faasm/faasm.h"

#include <stdio.h>

class MyClass{
public:
    explicit MyClass(int newInt) {
        myInt = newInt;
    }
    int myInt = 0;
};

void setMyClassPointer(MyClass **ptrPtr, int newInt) {
    MyClass *newPtr;

    newPtr = new MyClass(newInt);

    *ptrPtr = newPtr;
}

namespace faasm {
    int exec(FaasmMemory *memory) {
        // Initially null pointer
        MyClass *aPtr;

        // Set to something
        MyClass **aPtrPtr = &aPtr;
        setMyClassPointer(aPtrPtr, 2);

        if(aPtr == NULL) {
            printf("Pointer pointer causes null\n");
            return 1;
        }

        if(aPtr->myInt == 2) {
            printf("Pointer pointer handled properly\n");
            return 0;
        }

        return 1;
    }
}
