cmake_minimum_required(VERSION 3.16)
project(pitchlens LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

find_package(OpenCV REQUIRED)
find_package(Threads REQUIRED)

add_executable(pitchlens
  src/main.cpp
  src/detector.cpp)

target_include_directories(pitchlens PRIVATE src ${OpenCV_INCLUDE_DIRS})
target_link_libraries(pitchlens PRIVATE ${OpenCV_LIBS} Threads::Threads)
target_compile_options(pitchlens PRIVATE -Wall -Wextra)
