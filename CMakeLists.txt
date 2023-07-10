# CMakeList.txt : CMake project for AccelerateMVCC, include source and define
# project specific logic here.
cmake_minimum_required (VERSION 3.8)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("AccelerateMVCC")

# Add source to this project's executable.
add_executable (AccelerateMVCC
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
