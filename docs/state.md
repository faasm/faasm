# State

All of a users' functions have access to shared state. This state is implemented as 
a simple key-value store and accessed through the [Faasm host interface](host_interface.md).
 
At the lowest level, these values are treated as byte arrays for language independence, and
ease of inter-operation with WebAssembly.

Faasm provides high-level wrappers around certain state functions, with a C++ example
looking something like:

```c++
#include "faasm/faasm.h"

FAASM_MAIN_FUNC() {
    const char *key = "my_state_key";

    // Read the state into a buffer
    long stateSize = 123;
    uint8_t *myState = new uint8_t[stateSize];
    faasmReadState(key, newState, stateSize);

    // Do something useful, modify state

    // Write the updated state
    faasmWriteState(key, myState, stateSize);

    return 0;
}
```

### Offset state

Faasm supports reading and writing subsections of byte arrays stored against keys in 
the key-value store.

For example, if I have an array of 100 bytes stored in memory, I can read bytes 15-20, then 
update them separately.

This can be useful for implementing distributed iterative algorithms.

### Uploading state

If you need to prepopulate state for your functions you can use the state upload endpoint. This can be called with:

```bash
# Format /s/<username>/<key>
curl http://localhost:8002/s/user123/my_key -X PUT -d "your data"
```

This state can then be accessed in your functions using the specified key. For larger state, you can also upload a file:

```bash
curl http://localhost:8002/s/user123/my_key -X PUT -T /tmp/my_state_file
```

Where `/tmp/my_state_file` contains binary data you wish to be inserted at your specified key.
