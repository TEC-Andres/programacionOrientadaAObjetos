cmake_minimum_required(VERSION 3.10)
project(SituacionProblema)

set(CMAKE_CXX_STANDARD 17)

# MinGW toolchain is required — OpenSSL is expected at this path
set(OPENSSL_ROOT_DIR "C:/msys64/mingw64" CACHE PATH "MinGW OpenSSL root")

include_directories(${CMAKE_SOURCE_DIR}/lib ${CMAKE_SOURCE_DIR}/assets)

find_package(OpenSSL REQUIRED)

add_compile_definitions(HAS_OPENSSL)

file(GLOB_RECURSE LIB_SOURCES
	lib/*.cpp
)

add_library(SituacionProblemaLib ${LIB_SOURCES})
target_link_libraries(SituacionProblemaLib PRIVATE OpenSSL::Crypto)

add_executable(SituacionProblema src/main.cpp)
target_link_libraries(SituacionProblema PRIVATE SituacionProblemaLib)

# Place final executable into a top-level __release__ directory inside the source tree
set(RELEASE_OUTPUT_DIR "${CMAKE_SOURCE_DIR}/__release__")

file(GLOB TEST_SOURCES src/environmentTesting/*.cpp)

foreach(TEST_FILE ${TEST_SOURCES})
    get_filename_component(TEST_NAME ${TEST_FILE} NAME_WE)
    add_executable(${TEST_NAME} ${TEST_FILE})
    target_link_libraries(${TEST_NAME} PRIVATE SituacionProblemaLib)
    set_target_properties(${TEST_NAME} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${RELEASE_OUTPUT_DIR}/environments"
        RUNTIME_OUTPUT_DIRECTORY_DEBUG "${RELEASE_OUTPUT_DIR}/environments"
        RUNTIME_OUTPUT_DIRECTORY_RELEASE "${RELEASE_OUTPUT_DIR}/environments"
        RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${RELEASE_OUTPUT_DIR}/environments"
        RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${RELEASE_OUTPUT_DIR}/environments"
    )
endforeach()

# Main executable goes to __release__; tests go to __release__/environments
set_target_properties(SituacionProblema PROPERTIES
	RUNTIME_OUTPUT_DIRECTORY "${RELEASE_OUTPUT_DIR}"
	RUNTIME_OUTPUT_DIRECTORY_DEBUG "${RELEASE_OUTPUT_DIR}"
	RUNTIME_OUTPUT_DIRECTORY_RELEASE "${RELEASE_OUTPUT_DIR}"
	RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${RELEASE_OUTPUT_DIR}"
	RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${RELEASE_OUTPUT_DIR}"
)
