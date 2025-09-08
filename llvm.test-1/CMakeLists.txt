# Tutorial:
#   https://www.kitware.com/import-cmake-the-experiment-is-over/

cmake_minimum_required(VERSION 3.28...3.29)
project(a.out CXX)

set(LIB_NAME ${PROJECT_NAME}-lib)

find_package(LLVM REQUIRED)
find_package(Clang REQUIRED)

# Turning off extensions avoids and issue with the clang 16 compiler
# clang 17 and greater can avoid this setting
set(CMAKE_CXX_EXTENSIONS OFF)

# set(CMAKE_CXX_FLAGS "-Wall -g3 -O0 -fno-rtti ${LLVM_COMPILE_FLAGS}")
# get_property(clang_targets DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY IMPORTED_TARGETS)
# message(STATUS "Available Clang targets: ${clang_targets}")


# Create a library
add_library(${LIB_NAME})

# Add the module file to the library
target_sources(${LIB_NAME}
  PUBLIC
    FILE_SET CXX_MODULES FILES
      standard.cxx
)

# Create an executable
add_executable(${PROJECT_NAME} main.cxx)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")
message(STATUS "Using ClangConfig.cmake in: ${Clang_DIR}")

# Add LLVM/Clang include directories
include_directories(${LLVM_INCLUDE_DIRS})
include_directories(${CLANG_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

# Link against Clang & LLVM libraries you actually use
# For clang-tidy–style tools, these are common:
target_link_libraries(${PROJECT_NAME}
  PRIVATE
    ${LIB_NAME}
    LLVM
    clang-cpp
)

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_23)
target_compile_features(${LIB_NAME} PUBLIC cxx_std_23)


set_target_properties(${PROJECT_NAME} PROPERTIES
  CXX_STANDARD 23
  CXX_STANDARD_REQUIRED ON
  CXX_EXTENSIONS OFF
)

set_target_properties(${LIB_NAME} PROPERTIES
  CXX_STANDARD 23
  CXX_STANDARD_REQUIRED ON
  CXX_EXTENSIONS OFF
)
