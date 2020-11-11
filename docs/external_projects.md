# Using Faasm with External Projects

To compile projects outside the Faasm source tree you need to install the Faasm 
library and emulator to your system. This allows you to compile and run natively.

```
inv libs.native
```

This installs to `/usr/local/faasm/install/`. You can link to `libfaasm` and 
`libemulator` from there, e.g. with a file at `hello.cpp`:

```cpp
#include <faasm/faasm.h>
#include <stdio.h>

int main(int argc, char* argv[]) {
    printf("Hi external\n");
    return 0;
}
```

You can do:

```bash
clang++ hello.cpp -I /usr/local/faasm/install/include -L/usr/local/faasm/install/lib -lfaasm -lemulator
./a.out
```

This should print `Hi external`.

## Compiling to wasm

To compile to wasm you need to set up the toolchain:

TBC
