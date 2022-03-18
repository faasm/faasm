# C++ support

All C++ work takes place in the `cpp` container, i.e.:

```bash
./bin/cli.sh cpp
```

From here you can compile, upload and invoke different functions defined in the
[Faasm C++ repo](https://github.com/faasm/cpp), or add your own.

If you want to upload C/C++ functions from other repos, you can look at the
[Faasm HTTP API](api.md).

## Compiling a C++ function

A simple hello world function is
[`demo/hello`](https://github.com/faasm/cpp/blob/main/func/demo/hello.cpp) which
just returns a hello message.

From the Faasm CLI, you can compile, upload and invoke the `hello.cpp`
function with:

```bash
inv compile demo hello
inv upload demo hello
inv invoke demo hello
```

You should then see the response `Hello faasm!`.

## Updating a function

You can edit the message in the `hello.cpp` file, then update and invoke again
with:

```bash
# Recompile and upload
inv compile demo hello
inv upload demo hello

# Flush - this is important to remove any cached versions of your function
inv flush

# Invoke again
inv invoke demo hello
```

# Writing functions

Faasm aims to be uninvasive, allowing code to run natively _and_ in a serverless
context. This means the simplest Faasm function looks like:

```c++
int main(int argc, char* argv[]) {
    // Do something

    return 0;
}
```

## C++ API

Faasm provides a simple C++ wrapper library around the [Faasm host
interface](host_interface.md).  Some of the methods in this wrapper are:

- `faasmGetInput()` - allows functions to retrieve their input data
- `faasmSetOutput()` - this allows functions to return output data to the caller
- `faasmChain()` - this allows one function to invoke others
- `faasmAwaitCall()` - waits for a chained function invocation to finish
- `faasmReadState()` and `writeState()` - allows functions to read/ write
  key/value state
- `faasmReadStateOffset()` and `faasmWriteStateOffset()` - allows functions to
  read/ write at specific points in existing state (e.g. updating a subsection
  of an array)

They are found in the [`cpp`
repo](https://github.com/faasm/cpp/tree/main/libfaasm).

## Chaining

"Chaining" is when one function makes a call to another function (which must be
owned by the same user).  There are two supported methods of chaining, one for
invoking totally separate Faasm functions, the other for automatically
parallelising functions in the same piece of code (useful for porting legacy
applications).

### Chaining a function

Multiple functions can be defined in the same file, invoke each other and await
results.

Functions can either be chained by passing another function's name, or by
passing a pointer to a function in the same file.

Examples:

- Chaining by name
  [`demo/chain_named_a`](https://github.com/faasm/cpp/blob/main/func/demo/chain_named_a.cpp),
  [`demo/chain_named_b`](https://github.com/faasm/cpp/blob/main/func/demo/chain_named_b.cpp),
  [`demo/chain_named_c`](https://github.com/faasm/cpp/blob/main/func/demo/chain_named_c.cpp).
- Chaining by function pointer
  [`demo/chain`](https://github.com/faasm/cpp/blob/main/func/demo/chain.cpp).

