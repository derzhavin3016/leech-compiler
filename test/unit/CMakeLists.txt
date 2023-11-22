cpmaddpackage(
  NAME
  googletest
  GITHUB_REPOSITORY
  google/googletest
  VERSION
  1.14.0
  OPTIONS
  "INSTALL_GTEST OFF")

list_dirs(UNIT_DIRS ${CMAKE_CURRENT_SOURCE_DIR})

set(TESTLIST)
include(GoogleTest)

macro(ljit_add_utest src)
  set(TEST_NAME "${src}_test")
  add_executable(${TEST_NAME} ${src})
  upd_tar_list(${TEST_NAME} TESTLIST)
  gtest_discover_tests(
    ${TEST_NAME}
    EXTRA_ARGS --gtest_color=yes
    PROPERTIES LABELS unit)
endmacro()

foreach(DIR ${UNIT_DIRS})
  add_subdirectory(${DIR})
endforeach()

message(STATUS "Collected unit tests: ${TESTLIST}")

foreach(TEST ${TESTLIST})
  target_link_libraries(${TEST} PRIVATE GTest::gtest_main)
  target_link_libraries(${TEST} PRIVATE ${LIBLIST})

  target_compile_features(${TEST} PRIVATE cxx_std_20)
  target_include_directories(${TEST} PRIVATE ${CMAKE_SOURCE_DIR}/include)
endforeach()
