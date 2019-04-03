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
    *ptrPtr = new MyClass(newInt);
}

namespace faasm {
    int exec(FaasmMemory *memory) {
        // Initially null pointer
        MyClass *a;

        // Set to something
        setMyClassPointer(&a, 2);

        if(a == nullptr) {
            printf("Pointer pointer causes null\n");
            return 1;
        }

        if(a->myInt == 2) {
            printf("Pointer pointer handled properly\n");
            return 0;
        }

        return 1;
    }
}
