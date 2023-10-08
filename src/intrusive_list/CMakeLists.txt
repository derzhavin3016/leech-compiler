add_library(intrusive_list INTERFACE)
target_sources(
  intrusive_list
  INTERFACE ${CMAKE_SOURCE_DIR}/include/intrusive_list/intrusive_list.hh)
