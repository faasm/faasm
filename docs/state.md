# State

All of a users' functions have access to shared state. This state is implemented as 
a simple key-value store and accessed at a low level through the [Faasm host interface](host_interface.md),
and through higher level language-specific libraries.

The Faasm runtime ensures this state is shared efficiently across the cluster, taking 
care of all underlying replication and synchronisation.
 
Under the hood state is stored as byte arrays, making it language-agnostic and easy to integrate 
with WebAssembly.

Faasm provides some simple wrappers around state operations, e.g. in C++:

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

and Python:

```python
from pyfaasm.code import faasm_main
from pyfaasm.state import get_state, set_state

@faasm_main
def my_func():
    # Read the state
    key = "myKey"    
    state_val = get_state(key)
    
    # Do something useful
    
    # Set an updated value
    set_state(key, state_val)    
```

### Offset state

When operating in parallel on larger state values, it may be unnecessary to load
the full value into memory for every function instance. For example, many functions 
operating in parallel on a large matrix may only access a few rows or columns each. 
In this scenario it's unnecessarily expensive and slow to load the full matrix into 
every function. 

To cater for this, Faasm state values are byte-addressable, i.e. each function can 
explicitly access only a subsection of the value, and the Faasm runtime will ensure 
only the necessary data is transferred. This can improve performance and reduce
cost in large data-intensive applications.

The low-level offset state operations are part of the 
[Faasm host interface](host_interface.md), and explained in more detail in 
[our paper](https://arxiv.org/abs/2002.09344).
