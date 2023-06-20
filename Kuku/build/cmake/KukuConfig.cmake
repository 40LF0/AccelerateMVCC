# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT license.

# Exports target Kuku::kuku
#
# Creates variables:
#   Kuku_FOUND : If either a static or a shared Kuku library was found
#   Kuku_STATIC_FOUND : If a static Kuku library was found
#   Kuku_SHARED_FOUND : If a shared Kuku library was found
#   Kuku_C_FOUND : If a Kuku C export library was found
#   Kuku_VERSION : The full version number
#   Kuku_VERSION_MAJOR : The major version number
#   Kuku_VERSION_MINOR : The minor version number
#   Kuku_VERSION_PATH : The patch version number
#   Kuku_BUILD_TYPE : The build type (e.g., "Release" or "Debug")
#   Kuku_DEBUG : Set to non-zero value if Kuku is compiled with extra debugging code


####### Expanded from @PACKAGE_INIT@ by configure_package_config_file() #######
####### Any changes to this file will be overwritten by the next CMake run ####
####### The input file was KukuConfig.cmake.in                            ########

get_filename_component(PACKAGE_PREFIX_DIR "${CMAKE_CURRENT_LIST_DIR}/../../../" ABSOLUTE)

macro(set_and_check _var _file)
  set(${_var} "${_file}")
  if(NOT EXISTS "${_file}")
    message(FATAL_ERROR "File or directory ${_file} referenced by variable ${_var} does not exist !")
  endif()
endmacro()

macro(check_required_components _NAME)
  foreach(comp ${${_NAME}_FIND_COMPONENTS})
    if(NOT ${_NAME}_${comp}_FOUND)
      if(${_NAME}_FIND_REQUIRED_${comp})
        set(${_NAME}_FOUND FALSE)
      endif()
    endif()
  endforeach()
endmacro()

####################################################################################

set(Kuku_FOUND FALSE)
set(Kuku_STATIC_FOUND FALSE)
set(Kuku_SHARED_FOUND FALSE)
set(Kuku_C_FOUND FALSE)

set(Kuku_VERSION 2.1.0)
set(Kuku_VERSION_MAJOR 2)
set(Kuku_VERSION_MINOR 1)
set(Kuku_VERSION_PATCH 0)

set(Kuku_BUILD_TYPE Release)
set(Kuku_DEBUG OFF)

# Add the current directory to the module search path
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR})

include(${CMAKE_CURRENT_LIST_DIR}/KukuTargets.cmake)

if(TARGET Kuku::kuku)
    set(Kuku_FOUND TRUE)
    set(Kuku_STATIC_FOUND TRUE)
endif()

if(TARGET Kuku::kuku_shared)
    set(Kuku_FOUND TRUE)
    set(Kuku_SHARED_FOUND TRUE)
endif()

if(TARGET Kuku::kukuc)
    set(Kuku_FOUND TRUE)
    set(Kuku_C_FOUND TRUE)
endif()

if(Kuku_FOUND)
    if(NOT Kuku_FIND_QUIETLY)
        message(STATUS "Kuku -> Version ${Kuku_VERSION} detected")
    endif()
    if(Kuku_DEBUG AND NOT Kuku_FIND_QUIETLY)
        message(STATUS "Performance warning: Kuku compiled in debug mode")
    endif()
    set(KUKU_TARGETS_AVAILABLE "Kuku -> Targets available:")

    if(Kuku_STATIC_FOUND)
        string(APPEND KUKU_TARGETS_AVAILABLE " Kuku::kuku")
    endif()
    if(Kuku_SHARED_FOUND)
        string(APPEND KUKU_TARGETS_AVAILABLE " Kuku::kuku_shared")
    endif()
    if(Kuku_C_FOUND)
        string(APPEND KUKU_TARGETS_AVAILABLE " Kuku::kukuc")
    endif()
    if(NOT Kuku_FIND_QUIETLY)
        message(STATUS ${KUKU_TARGETS_AVAILABLE})
    endif()
else()
    if(NOT Kuku_FIND_QUIETLY)
        message(WARNING "Kuku -> NOT FOUND")
    endif()
endif()
