cmake_minimum_required(VERSION 3.14)

project(AccelerateMVCC)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

# GoogleTest requires at least C++14
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Common source files
set(COMMON_SOURCES
  "main.cpp"
  "include/accelerateMvcc.h"
  "include/trxManager.cpp"
  "include/epochList.h"
  "include/epochList.cpp"
  "include/node.h"
  "include/node.cpp"
  "include/common.h"
  "include/dead0dead.h"
  "include/read0read.h"
  "include/read0types.h"
  "include/ut0lst.h"
  "include/dead0dead.cpp"
  "include/read0read.cpp"
  "include/read0types.cpp"
)

# Executable target
add_executable(AccelerateMVCC ${COMMON_SOURCES})

# Include directories
target_include_directories(AccelerateMVCC
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Kuku/src
)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET AccelerateMVCC PROPERTY CXX_STANDARD 20)
endif()

# Link libraries
target_link_libraries(AccelerateMVCC
  ${CMAKE_CURRENT_SOURCE_DIR}/Kuku/build/lib/Debug/kuku-2.1.lib
)

# Tests
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

# Test executable target
add_executable(hello_test ${COMMON_SOURCES} "hello_test.cpp")

# Include directories for test target
target_include_directories(hello_test
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Kuku/src
)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET hello_test PROPERTY CXX_STANDARD 20)
endif()

# Link libraries for test target
target_link_libraries(hello_test
  ${CMAKE_CURRENT_SOURCE_DIR}/Kuku/build/lib/Debug/kuku-2.1.lib
  GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(hello_test)
