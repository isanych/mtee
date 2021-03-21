cmake_minimum_required(VERSION 3.19)
project(mtee)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)
set(CMAKE_CXX_STANDARD 14)
if(MSVC)
  string(REGEX REPLACE "/GR" "/GR-" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
  string(REGEX REPLACE "/EHsc" "/EHs-c-" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
else()
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti -fno-exceptions")
endif()

add_executable(mtee
  args.cpp
  cpuload.cpp
  cpuload.h
  handler.cpp
  header.h
  help.cpp
  helpers.cpp
  icon1.ico
  LICENSE.txt
  main.cpp
  output.cpp
  perr.cpp
  README.md
)
#target_compile_definitions(mtee PRIVATE UNICODE _UNICODE)