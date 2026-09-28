cmake_minimum_required(VERSION 3.16)
project(bounce C)

find_package(SDL2 REQUIRED CONFIG)

add_executable(bounce src/main.c src/audio.c)
if(TARGET SDL2::SDL2main)
  target_link_libraries(bounce PRIVATE SDL2::SDL2main)
endif()
target_link_libraries(bounce PRIVATE SDL2::SDL2)
