list_dirs(DIRS ${CMAKE_CURRENT_SOURCE_DIR})
set(TOOLLIST ${DIRS})

foreach(DIR ${DIRS})
  add_subdirectory(${DIR})
  message("Included subdirectory: ${DIR}")
endforeach()

foreach(TOOL ${TOOLLIST})
  target_link_libraries(${TOOL} PRIVATE CLI11::CLI11)
  target_include_directories(
    ${TOOL} SYSTEM PRIVATE ${CMAKE_SOURCE_DIR}/thirdparty/CLI11/include)

  set_target_properties(${TOOL} PROPERTIES RUNTIME_OUTPUT_DIRECTORY
                                           "${CMAKE_BINARY_DIR}/bin/")
endforeach()

set(TOOLLIST
    "${TOOLLIST}"
    PARENT_SCOPE)
