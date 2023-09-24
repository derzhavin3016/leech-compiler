# Lookup for gtest
find_package(GTest REQUIRED)

list_dirs(UNIT_DIRS ${CMAKE_CURRENT_SOURCE_DIR})

set(TESTLIST)

foreach(DIR ${UNIT_DIRS})
  add_subdirectory(${DIR})
endforeach()

message(STATUS "Collected unit tests: ${TESTLIST}")

foreach(TEST ${TESTLIST})
  target_include_directories(${TEST} PRIVATE ${GTEST_INCLUDE_DIRS}
                                             ${CMAKE_CURRENT_SOURCE_DIR})
  target_link_libraries(${TEST} PRIVATE ${GTEST_BOTH_LIBRARIES} pthread)
  target_link_libraries(${TEST} PRIVATE ${LIBLIST})

  target_compile_features(${TEST} PRIVATE cxx_std_20)
  target_include_directories(${TEST} PRIVATE ${CMAKE_SOURCE_DIR}/include)

  add_test(NAME ${TEST} COMMAND ${TEST} --gtest_color=yes)
  set_property(TEST ${TEST} PROPERTY LABELS unit)
endforeach()
