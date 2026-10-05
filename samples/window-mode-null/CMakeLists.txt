cmake_minimum_required(VERSION 3.16)
project(pacer C)

find_package(SDL2 REQUIRED CONFIG)

add_executable(pacer src/main.c)
if(TARGET SDL2::SDL2main)
  target_link_libraries(pacer PRIVATE SDL2::SDL2main)
endif()
target_link_libraries(pacer PRIVATE SDL2::SDL2)
