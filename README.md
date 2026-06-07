# Situación Problema - Programación Orientada a Objetos

## Build 
```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

## Requirements 
- C++17 compatible compiler
- CMake 3.10 or higher
- VSC SDK for C++ (optional, for development)
- Windows 10 or higher (for console features)

## Compatibility table
| Shell provider | Tested status | Is env functional? | Comments |
| :--- | :--- | :--- | :--- |
| Windows comcast | YES | YES | - |
| Windows Terminal | YES | YES | Small garbage bug still persists causing sidebar to show up in env. But it now initializes at the correct size |
| MINGW64 | YES | NO | UTF-16 characters are not loaded correctly & character build-up occurs |
| VSC Terminal | YES | YES | Small garbage bug still persists causing sidebar to show up in env. But it now initializes at the correct size |
| MacOS Terminal | YES | NO | UTF-16 issue |
| MacOS VSC Terminal | YES | YES | Small garbage bug still persists causing sidebar to show up in env. But it now initializes at the correct size |
