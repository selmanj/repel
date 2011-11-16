add_executable(atomtest EXCLUDE_FROM_ALL atomtest.cpp)

target_link_libraries(atomtest ${pel_LIBRARIES} pel-fol pel-spaninterval)
