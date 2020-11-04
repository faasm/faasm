#include "faasm/faasm.h"

#include <stdio.h>

class MyClass
{
  public:
    explicit MyClass(int newInt) { myInt = newInt; }

    int myInt = 0;
};

void setMyClassPointer(MyClass** ptrPtr, int newInt)
{
    MyClass* newPtr;
    newPtr = new MyClass(newInt);

    if (ptrPtr == NULL) {
        printf("Pointer to pointer is null A\n");
    }

    *ptrPtr = newPtr;

    if (ptrPtr == NULL) {
        printf("Pointer to pointer is null B\n");
    }
}

FAASM_MAIN_FUNC()
{
    // Create a pointer and a pointer to a pointer
    MyClass* aPtr;
    MyClass** aPtrPtr = &aPtr;

    setMyClassPointer(aPtrPtr, 2);

    if (aPtr == NULL) {
        printf("Pointer pointer causes null\n");
        return 1;
    }
    if (aPtrPtr == NULL) {
        printf("Pointer to pointer is null\n");
    }

    if (aPtr->myInt == 2) {
        printf("Pointer pointer handled properly\n");
        return 0;
    }

    return 1;
}
