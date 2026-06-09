# Environment Testing
In order to test different environments, just change on CMakeLists.txt the line that says:
```cmake
add_executable(SituacionProblema src/main.cpp)
```

to

```cmake
add_executable(SituacionProblema src/environmentTesting/script.cpp)
```

This will allow you to test the environment and its compatibility with the project. The script.cpp file contains code that tests the environment and its features, such as console output, file handling, and other functionalities that may be relevant for the project.