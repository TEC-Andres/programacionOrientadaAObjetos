# Compilation Process

## environment prerequisites
before starting the build process, make sure you have installed the following tools:
* a native compiler with full support for the c++17 standard
* cmake build system version 3.10 or higher
* openssl external development libraries

## compilation steps
follow this simple sequence of commands in your terminal to compile the code:

### step 1:generate configurations
run this command to set up the build directory:
```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
```
### step 2: run the compilation engine to generate the final binaries.

```
cmake --build build --config Release
```