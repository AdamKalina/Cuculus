cmake_minimum_required(VERSION 3.14)
set(CMAKE_TRY_COMPILE_TARGET_TYPE "STATIC_LIBRARY") #https://stackoverflow.com/questions/53633705/cmake-the-c-compiler-is-not-able-to-compile-a-simple-test-program

project(Cuculus LANGUAGES C CXX) # here must be C CXX because of edflib.c

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 COMPONENTS Core REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core REQUIRED)

# for Win XP
if(${QT_VERSION_MAJOR} EQUAL 5 AND ${QT_VERSION_MINOR} LESS_EQUAL 6)
    SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS,5.01")
    message(STATUS "XP only")
else()

endif()

add_executable(Cuculus
  main.cpp
  read_signal_file.cpp
  edflib.c
  write_edf_file.cpp
)
target_link_libraries(Cuculus Qt${QT_VERSION_MAJOR}::Core)

add_definitions(-D_LARGEFILE64_SOURCE -D_LARGEFILE_SOURCE)
