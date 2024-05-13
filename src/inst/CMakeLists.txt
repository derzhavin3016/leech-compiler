add_library(inst inst.cc)
target_include_directories(inst PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_compile_features(inst PRIVATE cxx_std_17)
target_sources(inst INTERFACE ${CMAKE_SOURCE_DIR}/include/ir/inst.hh)
