# CMakeList.txt : CMake project for AccelerateMVCC, include source and define
# project specific logic here.
cmake_minimum_required(VERSION 3.14)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

# GoogleTest requires at least C++14
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project ("AccelerateMVCC")
# Common source files
set(COMMON_SOURCES
  "include/accelerateMvcc.h"
  "include/accelerateMvcc.cpp"
  "include/trxManager.cpp"
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
  "include/epoch_table.h"
  "include/epoch_table.cpp"
)
# Add source to this project's executable.
add_executable (AccelerateMVCC
  "main.cpp"
  ${COMMON_SOURCES}
 )


# Add the directory containing kuku/kuku.h and kuku/common.h to the include directories
target_include_directories(AccelerateMVCC
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Kuku/src
)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET AccelerateMVCC PROPERTY CXX_STANDARD 20)
endif()

# Link the kuku.lib library
target_link_libraries(AccelerateMVCC
  ${CMAKE_CURRENT_SOURCE_DIR}/Kuku/build/lib/Debug/kuku-2.1.lib
)

# TODO: Add tests and install targets if needed.
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(
  test_with_google
  "google_test.cpp"
  ${COMMON_SOURCES} 
)

# Add the directory containing kuku/kuku.h and kuku/common.h to the include directories
target_include_directories(test_with_google
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Kuku/src
)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET AccelerateMVCC PROPERTY CXX_STANDARD 20)
endif()

# Link the kuku.lib library
target_link_libraries(test_with_google
  ${CMAKE_CURRENT_SOURCE_DIR}/Kuku/build/lib/Debug/kuku-2.1.lib
)
target_link_libraries(
  test_with_google
  GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(test_with_google)