cpmaddpackage(
  NAME
  CLI11
  GITHUB_REPOSITORY
  CLIUtils/CLI11
  VERSION
  2.3.2
  OPTIONS
  "CLI11_PRECOMPILED ON"
  EXCLUDE_FROM_ALL
  TRUE
  SYSTEM
  TRUE)

list_dirs(DIRS ${CMAKE_CURRENT_SOURCE_DIR})
set(TOOLLIST ${DIRS})

foreach(DIR ${DIRS})
  add_subdirectory(${DIR})
  message("Included subdirectory: ${DIR}")
endforeach()

foreach(TOOL ${TOOLLIST})
  target_link_libraries(${TOOL} PRIVATE CLI11::CLI11)

  set_target_properties(${TOOL} PROPERTIES RUNTIME_OUTPUT_DIRECTORY
                                           "${CMAKE_BINARY_DIR}/bin/")
endforeach()

set(TOOLLIST
    "${TOOLLIST}"
    PARENT_SCOPE)
