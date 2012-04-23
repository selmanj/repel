add_subdirectory(syntax)

add_library(pel-logic
  Domain.cpp
  FOLLexer.cpp
  FOLToken.cpp
  Model.cpp
  Moves.cpp
  NameGenerator.cpp
  UnitProp.cpp
  ../inference/MCSat.cpp
  ../inference/MaxWalkSat.cpp
  ../inference/LiquidSampler.cpp
  ../inference/MCSatSamplePerfectlyStrategy.cpp)

target_link_libraries(pel-logic pel-syntax)