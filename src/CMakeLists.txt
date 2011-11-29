add_subdirectory(fol)

add_library(pel-spaninterval
  interval.cpp
  spaninterval.cpp
  siset.cpp)

# set_target_properties(pel-spaninterval PROPERTIES COMPILE_FLAGS "-O3 -Wall")

set (pel_SOURCES siset.cpp)
set (pel_LIBRARIES ${pel_LIBRARIES} pel-fol pel-spaninterval)

add_executable(pelmap pel.cpp ${pel_SOURCES})

# set_target_properties(pel PROPERTIES COMPILE_FLAGS "-O3 -Wall")

target_link_libraries(pelmap ${pel_LIBRARIES})

