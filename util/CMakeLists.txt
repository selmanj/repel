project(util)

add_executable(volleyball-gen volleyball-gen.cpp)
add_executable(volleyball-mcsat volleyball-mcsat.cpp)
target_link_libraries(volleyball-mcsat ${Boost_SERIALIZATION_LIBRARY} ${Boost_PROGRAM_OPTIONS_LIBRARY} pel-logic pel-syntax pel-spaninterval )