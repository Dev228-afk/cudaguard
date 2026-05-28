cmake_minimum_required(VERSION 3.20)
project(CudaGuard
    VERSION 1.0.0
    LANGUAGES CXX
    DESCRIPTION "CUDA C++ Static Analysis & Build Diagnostics Tool"
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Find LLVM and Clang
find_package(LLVM REQUIRED CONFIG)
find_package(Clang REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")
message(STATUS "Using ClangConfig.cmake in: ${Clang_DIR}")

list(APPEND CMAKE_MODULE_PATH "${LLVM_CMAKE_DIR}")
include(AddLLVM)

# LLVM definitions and include directories
separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND ${LLVM_DEFINITIONS})
add_definitions(${LLVM_DEFINITIONS_LIST})

# Core library
add_library(cudaguard_core
    src/Diagnostics.cpp
    src/RuleRegistry.cpp
    src/ToolConfig.cpp
    src/CompileDatabaseLoader.cpp
    src/BuildWrapper.cpp
    src/rules/MissingCudaErrorCheckRule.cpp
    src/rules/HostDeviceCallRule.cpp
    src/rules/KernelLaunchRule.cpp
    src/rules/SharedMemoryRule.cpp
    src/rules/CudaMallocMemcpyRule.cpp
)

target_include_directories(cudaguard_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${LLVM_INCLUDE_DIRS}
    ${CLANG_INCLUDE_DIRS}
)

target_link_libraries(cudaguard_core PUBLIC
    clangTooling
    clangASTMatchers
    clangFrontend
    clangSerialization
    clangDriver
    clangParse
    clangSema
    clangAnalysis
    clangAST
    clangBasic
    clangEdit
    clangLex
    clangRewrite
    LLVM
)

target_compile_features(cudaguard_core PUBLIC cxx_std_20)

# CLI executable
add_executable(cudaguard src/main.cpp)
target_link_libraries(cudaguard PRIVATE cudaguard_core)

# Install
install(TARGETS cudaguard RUNTIME DESTINATION bin)

# Testing
option(CUDAGUARD_BUILD_TESTS "Build unit tests" ON)
if(CUDAGUARD_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
