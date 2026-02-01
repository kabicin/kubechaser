cmake_minimum_required(VERSION 3.1)
project(stb_image)

target_include_directories(stb_image PUBLIC include)

if(NOT WIN32)
  target_link_libraries(stb_image PUBLIC ${CMAKE_DL_LIBS})
endif()