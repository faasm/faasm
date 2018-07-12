# Compatibility

Wavm will always stub out unrecognised functions and throw an "unreachable" exception if they are actually
executed.

## Breakdown

All of the following features will compile but some will work and others will throw an "unreachable" exception.

Note that a lot of them will work through executing `function.js`.

_Working_

- `printf`
- `malloc`/ `free`

_Unreachable_

- `sockets`
- `system`
- `pthread_create`
- `fopen`
- `fork`
- `exit`
