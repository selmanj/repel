add_subdirectory(logic)

add_library(pel-spaninterval
  interval.cpp
  spaninterval.cpp
  siset.cpp)

# set_target_properties(pel-spaninterval PROPERTIES COMPILE_FLAGS "-O3 -Wall")

set (pel_SOURCES siset.cpp)
set (pel_LIBRARIES ${pel_LIBRARIES} pel-logic pel-syntax pel-spaninterval)

add_executable(pel pel.cpp ${pel_SOURCES})

# set_target_properties(pel PROPERTIES COMPILE_FLAGS "-O3 -Wall")

target_link_libraries(pel ${pel_LIBRARIES})
install(TARGETS pel 
        RUNTIME DESTINATION bin)