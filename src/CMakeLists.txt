add_subdirectory(logic)

add_library(pel-spaninterval
  Interval.cpp
  SpanInterval.cpp
  SISet.cpp)

# set_target_properties(pel-spaninterval PROPERTIES COMPILE_FLAGS "-O3 -Wall")

set (pel_SOURCES SISet.cpp)
set (pel_LIBRARIES ${pel_LIBRARIES} pel-logic pel-syntax pel-spaninterval)

add_executable(pel PEL.cpp ${pel_SOURCES})

# set_target_properties(pel PROPERTIES COMPILE_FLAGS "-O3 -Wall")

target_link_libraries(pel ${pel_LIBRARIES})
install(TARGETS pel 
        RUNTIME DESTINATION bin)