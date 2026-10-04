cmake_minimum_required(VERSION 3.16)
project(soundboard C)

find_package(SDL2 REQUIRED CONFIG)

add_executable(soundboard src/main.c)
if(TARGET SDL2::SDL2main)
  target_link_libraries(soundboard PRIVATE SDL2::SDL2main)
endif()
target_link_libraries(soundboard PRIVATE SDL2::SDL2)
