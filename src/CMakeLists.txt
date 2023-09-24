list_dirs(DIRS ${CMAKE_CURRENT_SOURCE_DIR})
set(LIBLIST ${DIRS})

foreach(DIR ${DIRS})
  add_subdirectory(${DIR})
  message("Included subdirectory: ${DIR}")
endforeach()

foreach(LIB ${LIBLIST})
  format_target(${LIB} ${CMAKE_CURRENT_SOURCE_DIR}/${LIB} ${LIB}.cc)
endforeach()

set(LIBLIST
    "${LIBLIST}"
    PARENT_SCOPE)
