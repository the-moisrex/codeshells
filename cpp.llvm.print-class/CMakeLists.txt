# Tutorial:
#   https://www.kitware.com/import-cmake-the-experiment-is-over/

cmake_minimum_required(VERSION 3.13)
project(clang-print-class-plugin)


# find_package(LLVM  CONFIG)
find_package(Clang  CONFIG)


message(STATUS "Found LLVM ${LLVM_DIR}")
message(STATUS "Found Clang ${Clang_DIR}")


set(CMAKE_CXX_STANDARD 26)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)


add_library(print-class-plugin SHARED main.cxx)


target_include_directories(print-class-plugin PRIVATE
${LLVM_INCLUDE_DIRS}
${CLANG_INCLUDE_DIRS}
)


# Link against the Clang libraries we used; adjust if link errors occur.
target_link_libraries(print-class-plugin PRIVATE
    clang-cpp
    # clangAST
    # clangFrontend
    # clangTooling
    # clangBasic
    # clangSerialization
)


# Some environments require these flags from llvm-config.
# llvm_map_components_to_libnames(llvm_libs support core)
# target_link_libraries(print-class-plugin PRIVATE ${llvm_libs})
