cmake_minimum_required(VERSION 3.16)
project(menu C)

find_package(SDL2 REQUIRED CONFIG)

add_executable(menu src/main.c)
if(TARGET SDL2::SDL2main)
  target_link_libraries(menu PRIVATE SDL2::SDL2main)
endif()
target_link_libraries(menu PRIVATE SDL2::SDL2)
