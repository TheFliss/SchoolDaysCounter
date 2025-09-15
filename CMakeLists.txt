cmake_minimum_required(VERSION 3.15)
set(CMAKE_GENERATOR_PLATFORM x64)
set(CMAKE_TRY_COMPILE_TARGET_TYPE EXECUTABLE)
set(SRC "${CMAKE_CURRENT_SOURCE_DIR}")
set(PROJECT_VERSION "L\"3.1.0\"")

# Compiler Options
set(CMAKE_CXX_STANDARD 23)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set(CMAKE_CONFIGURATION_TYPES Release MinSizeRel Debug)
set(CMAKE_BUILD_TYPE Release)
project(SchoolDaysCounter CXX C RC)
foreach(LANG C CXX RC)
    set(CMAKE_${LANG}_STANDARD_INCLUDE_DIRECTORIES ${CUSTOM_INCLUDE_DIRECTORIES})
endforeach()

# Configure to build 
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${SRC}/builds")

file(GLOB_RECURSE ${PROJECT_NAME}_All
  ${SRC}/src/*.cpp
  ${SRC}/src/*.c
  ${SRC}/src/*.hpp
  ${SRC}/src/*.h
  ${SRC}/src/*.rc
)

add_executable(${PROJECT_NAME} WIN32 ${${PROJECT_NAME}_All})
set_property(TARGET ${PROJECT_NAME} PROPERTY
  MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

target_include_directories(${PROJECT_NAME} PUBLIC
  ${SRC}/src
  ${SRC}/include
)

target_link_libraries(${PROJECT_NAME} PRIVATE 
  Shlwapi.lib
  Msimg32.lib
  Comctl32.lib
  UxTheme.lib
)

target_compile_definitions(${PROJECT_NAME} PRIVATE $<$<CONFIG:Debug>:DEBUG> PROJECT_VERSION="${PROJECT_VERSION}")

target_compile_options(${PROJECT_NAME} PRIVATE
  $<$<COMPILE_LANGUAGE:CXX>:/MP>
  $<$<COMPILE_LANGUAGE:CXX>:/MT>
  #$<$<COMPILE_LANGUAGE:CXX>:/MD>
  $<$<COMPILE_LANGUAGE:CXX>:/W4>
)