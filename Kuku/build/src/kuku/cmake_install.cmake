# Install script for directory: C:/Users/wjdac/source/repos/AccelerateMVCC/Kuku/src/kuku

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "C:/Program Files (x86)/Kuku")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Release")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/Kuku-2.1/kuku/internal" TYPE FILE FILES
    "C:/Users/wjdac/source/repos/AccelerateMVCC/Kuku/src/kuku/internal/blake2.h"
    "C:/Users/wjdac/source/repos/AccelerateMVCC/Kuku/src/kuku/internal/blake2-impl.h"
    "C:/Users/wjdac/source/repos/AccelerateMVCC/Kuku/src/kuku/internal/hash.h"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/Kuku-2.1/kuku" TYPE FILE FILES
    "C:/Users/wjdac/source/repos/AccelerateMVCC/Kuku/src/kuku/common.h"
    "C:/Users/wjdac/source/repos/AccelerateMVCC/Kuku/src/kuku/kuku.h"
    "C:/Users/wjdac/source/repos/AccelerateMVCC/Kuku/src/kuku/locfunc.h"
    )
endif()

