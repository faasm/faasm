# Demo Slides

1. Overview

- More of a concrete display of what Faasm looks like and how it fits with other stuff
- Github link

2. Components

- Storage (object store/ filesystem)
- Worker (Just multithreaded C++) - needs access to storage
- Upload (Just C++) - needs access to storage
- Redis (KV/ state)

Others

- Edge
- Redis (queue)

How does this fit in other places?

- Run the worker (e.g. on VMs with lots of cores, in "fat" functions etc.). As long as it autoscales
- Write 3 adapters:
- One to listen to the relevant queue
- One to interface with the storage
- One to interact with the KV store for state

3. Docker Compose Demo

- Docker compose file, show contents/ containers
- Show in action, go through README?
- Show Python file in action

What to show:

- Function source
- Upload the function
- objdump -t function.wasm.o
- WAST on wasm file
- Upload python

3. Work sharing

How it works

4. Multi-language support

Emphasise latest LLVM supports this (can download now). All browsers support this.

- Out of the box - C/C++, Typescript, Rust, Go
- Via interpreter - Python (Ruby?)
- Future LLVM front-ends?

Faasm library integrations:

- C/ C++ and Rust in form of headers
- Python library with C FFI

