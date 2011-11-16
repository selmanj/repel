set (pel_SOURCES siset.cpp)
set (pel_SOURCES ${pel_SOURCES} interval.cpp)
set (pel_SOURCES ${pel_SOURCES} spaninterval.cpp)
set (pel_SOURCES ${pel_SOURCES} fol/boollit.cpp)
set (pel_SOURCES ${pel_SOURCES} fol/constant.cpp)
set (pel_SOURCES ${pel_SOURCES} fol/domain.cpp)
set (pel_SOURCES ${pel_SOURCES} fol/formulaset.cpp)
set (pel_SOURCES ${pel_SOURCES} fol/follexer.cpp)
set (pel_SOURCES ${pel_SOURCES} fol/foltoken.cpp)
set (pel_SOURCES ${pel_SOURCES} fol/model.cpp)
set (pel_SOURCES ${pel_SOURCES} fol/moves.cpp)
set (pel_SOURCES ${pel_SOURCES} fol/namegenerator.cpp)
set (pel_SOURCES ${pel_SOURCES} fol/variable.cpp)

add_executable(pel pel.cpp ${pel_SOURCES})

set_target_properties(pel PROPERTIES COMPILE_FLAGS "-O3 -Wall")

target_link_libraries(pel ${pel_LIBRARIES})

