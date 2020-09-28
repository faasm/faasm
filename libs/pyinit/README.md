# Faasm Python C Library

This is a small library to force the static linking of certain parts of libc
that need to be present to support numpy and other mathematical Python 
libraries. 

Without this library the functions would be stripped from the underlying
WebAssembly binary (as they're only used by dynamically linked libraries which
are not known at compile time).

