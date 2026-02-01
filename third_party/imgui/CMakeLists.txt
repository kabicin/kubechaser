cmake_minimum_required(VERSION 3.6)
project(imgui)

file(GLOB SOURCES src/*.cpp)

add_library(imgui ${SOURCES})

target_include_directories(imgui PUBLIC include)

if(NOT WIN32)
  target_link_libraries(imgui PUBLIC ${CMAKE_DL_LIBS})
endif()

target_link_libraries(imgui glad glfw)
