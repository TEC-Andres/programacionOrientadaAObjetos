# compilation process

## Environment prerequisites
before starting the build process, make sure you have installed the following tools:
* A native compiler with full support for the C++17 standard  
* CMAKE build system version 3.10 or higher  
* OpenSSL external development libraries  

## Compilation steps
Follow this simple sequence of commands in your terminal to compile the code:

## Requirements
The following requirements must be met before compilation:
* A C++17 compliant compiler
* CMake 3.10 or higher
* OpenSSL development libraries
* SQLite3 development libraries

Preferably for windows, use Visual Studio Code SDK for C++ or the mingw-w64 toolchain for compilation.

### Step 1: Generate configurations
Run this command to set up the build directory:
```bash
cmake -S . -B build -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release
```
### Step 2: Run the compilation engine to generate the final binaries.

```bash
cmake --build build --config Release
```

What you will end up seeing is a list of binaries in the `__release__` folder.  
```
C:.
│   SituacionProblema.exe
│
└───environments
        eccDatabase.exe
        eccTest.exe
        login.exe
        videoClass.exe
```

