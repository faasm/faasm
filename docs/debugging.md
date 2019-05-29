# Debugging WASM

## Symbols

To understand the output of gdb we will need the function symbols available, which we can do with
the `func_sym` executable:

```
func_sym python py_func
```

## GDB

You can use `gdb` to debug wasm functions as follows. If we have the function from user `python`
called `py_func`, we can run this in isolation with the `func_runner` executable, e.g.

```
# Normally
func_runner python py_func

# GDB
gdb --args func_runner python py_func
```

Because the function itself is loaded with the LLVM JIT libraries, GDB doesn't have the symbols
up front, but if we know the function from the symbols output we can set a breakpoint pending shared
library load.

```
break functionDef1234
```

You can then use normal gdb functionality, albeit with a lack of source information, e.g. view backtraces
inspect stack frames etc.