add_subdirectory(syntax)

add_library(pel-logic
  domain.cpp
  follexer.cpp
  foltoken.cpp
  model.cpp
  moves.cpp
  namegenerator.cpp
  unit_prop.cpp
  ../inference/mcsat.cpp
  ../inference/maxwalksat.cpp)

target_link_libraries(pel-logic pel-syntax)