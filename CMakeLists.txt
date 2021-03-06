cmake_minimum_required(VERSION 2.6)
project (PEL)

set (PEL_VERSION_MAJOR 0)
set (PEL_VERSION_MINOR 8)

# force error messages to be one line
IF(CMAKE_COMPILER_IS_GNUCC)
  SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fmessage-length=0")
ENDIF(CMAKE_COMPILER_IS_GNUCC)
IF(CMAKE_COMPILER_IS_GNUCXX)
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fmessage-length=0")
ENDIF(CMAKE_COMPILER_IS_GNUCXX)

include_directories("${PROJECT_BINARY_DIR}/src")
#set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

set(Boost_USE_STATIC_LIBS   OFF)
set(Boost_USE_MULTITHREADED OFF)
#set(Boost_ADDITIONAL_VERSIONS "1.46.1" "1.47" "1.47.0" "1.48" "1.48.0")

find_package(Boost 1.48.0 COMPONENTS program_options iostreams unit_test_framework serialization)

if (NOT Boost_USE_STATIC_LIBS AND Boost_UNIT_TEST_FRAMEWORK_FOUND)
	set(USE_DYNAMIC_UNIT_TEST ON)
else()
	set(USE_DYNAMIC_UNIT_TEST OFF)
endif()

configure_file (
  "${PROJECT_SOURCE_DIR}/src/config.h.in"
  "${PROJECT_BINARY_DIR}/src/config.h"
  )

include_directories(${Boost_INCLUDE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/src)

set(pel_LIBRARIES ${Boost_PROGRAM_OPTIONS_LIBRARY} ${Boost_SERIALIZATION_LIBRARY})

enable_testing()

add_subdirectory(src)
add_subdirectory(test)
add_subdirectory(util EXCLUDE_FROM_ALL)
