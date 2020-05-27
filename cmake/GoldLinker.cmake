include(CMakeDependentOption)
cmake_dependent_option(ENABLE_LD_GOLD "Use GNU gold linker" ON "UNIX" OFF)

set(LD_GOLD_FOUND FALSE)
if(ENABLE_LD_GOLD)
  execute_process(COMMAND ${CMAKE_C_COMPILER} -fuse-ld=gold -Wl,--version ERROR_QUIET OUTPUT_VARIABLE LD_VERSION)
  if(LD_VERSION MATCHES "GNU gold")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fuse-ld=gold")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fuse-ld=gold")
    set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fuse-ld=gold")
    set(LD_GOLD_FOUND TRUE)
    message(STATUS "Linker: GNU gold")
  else()
    message(WARNING "GNU gold linker is not available, falling back to default system linker")
  endif()
else()
endif()
