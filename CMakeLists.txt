cmake_minimum_required(VERSION 2.6)
project (PEL)

set (PEL_VERSION_MAJOR 0)
set (PEL_VERSION_MINOR 8)

configure_file (
  "${PROJECT_SOURCE_DIR}/src/PELConfig.h.in"
  "${PROJECT_BINARY_DIR}/src/PELConfig.h"
  )

include_directories("${PROJECT_BINARY_DIR}/src")
#set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

find_package(Boost COMPONENTS program_options REQUIRED)

include_directories(${Boost_INCLUDE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/src)

set(pel_LIBRARIES ${Boost_LIBRARIES})

enable_testing()

add_subdirectory(src)
add_subdirectory(test)

