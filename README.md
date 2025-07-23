# cheri-thunk
Library for fun experiments with CHERI and code thunks.

## Building

```
cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=cmake/morello-toolchain.cmake -DCHERI_SDK=path/to/cherisdk -DCMAKE_SYSROOT=path/to/cherisdk/rootfs-morello-purecap
```

