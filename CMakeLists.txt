cmake_minimum_required(VERSION 3.25)
project(sdl3_porter_fixtures LANGUAGES C)

# SDL3 enables Objective-C only in its own directory, which leaves SDL2's .m
# sources without an Objective-C compile rule. Enabling it here covers both.
if(APPLE)
  enable_language(OBJC)
endif()

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

include(FetchContent)

# Static SDL: nothing to copy next to the fixtures on Windows.
set(SDL_SHARED OFF CACHE BOOL "" FORCE)
set(SDL_STATIC ON CACHE BOOL "" FORCE)
set(SDL_TEST_LIBRARY OFF CACHE BOOL "" FORCE)
set(SDL_EXAMPLES OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
  SDL3
  URL https://github.com/libsdl-org/SDL/releases/download/release-3.4.16/SDL3-3.4.16.tar.gz
  URL_HASH SHA256=7322236cd12090c3eb40b9728be4d49c76f66ad17d04369584d4ecad5cf77c68
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(SDL3)

# The SDL2 originals, to prove each fixture's check passes on SDL2 behavior.
# SDL2 reads SDL3's cached options, so it also builds static and with the C
# library. SDL_TEST is SDL2's name for SDL_TEST_LIBRARY.
set(SDL_TEST OFF CACHE BOOL "" FORCE)
FetchContent_Declare(
  SDL2
  URL https://github.com/libsdl-org/SDL/releases/download/release-2.32.10/SDL2-2.32.10.tar.gz
  URL_HASH SHA256=5f5993c530f084535c65a6879e9b26ad441169b3e25d789d83287040a9ca5165
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(SDL2)

enable_testing()

set(run_naive "${CMAKE_CURRENT_SOURCE_DIR}/cmake/run-naive.cmake")

# One test per program. The warning flags enforce "compiles cleanly": a naive
# port the compiler warns about is guidance, not a trap.
function(add_fixture id variant)
  set(target "${id}-${variant}")
  add_executable(${target} "fixtures/${id}/${variant}/main.c")
  if(variant STREQUAL "sdl2")
    target_link_libraries(${target} PRIVATE SDL2::SDL2main SDL2::SDL2)
    # SDL2 spells the driver variables without the second underscore.
    set(drivers "SDL_VIDEODRIVER=dummy;SDL_AUDIODRIVER=dummy")
  else()
    target_link_libraries(${target} PRIVATE SDL3::SDL3)
    set(drivers "SDL_VIDEO_DRIVER=dummy;SDL_AUDIO_DRIVER=dummy")
  endif()
  if(MSVC)
    target_compile_options(${target} PRIVATE /W4 /WX)
  else()
    target_compile_options(${target} PRIVATE -Wall -Wextra -Werror)
  endif()
  if(variant STREQUAL "naive")
    # The positive control: passes only if the naive port fails with its trap.
    add_test(NAME ${target}
      COMMAND ${CMAKE_COMMAND} -DEXE=$<TARGET_FILE:${target}> -DID=${id} -P ${run_naive}
    )
  else()
    add_test(NAME ${target} COMMAND ${target})
  endif()
  set_tests_properties(${target} PROPERTIES
    ENVIRONMENT "${drivers}"
    TIMEOUT 30
  )
endfunction()

# The SDL files bundled with the skill must match this release byte for byte.
set(sdl_bundle "${CMAKE_CURRENT_SOURCE_DIR}/skills/sdl3-porter/scripts/sdl")
add_test(NAME bundled-sdl-files
  COMMAND ${CMAKE_COMMAND} -DBUNDLE_DIR=${sdl_bundle} -DSDL_SOURCE_DIR=${sdl3_SOURCE_DIR}
          -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/check-bundled-sdl.cmake
)

# Positive control: the same check must fail on a tampered copy.
set(tampered_bundle "${CMAKE_CURRENT_BINARY_DIR}/tampered-sdl-bundle")
file(REMOVE_RECURSE "${tampered_bundle}")
file(COPY "${sdl_bundle}/" DESTINATION "${tampered_bundle}")
file(APPEND "${tampered_bundle}/build-scripts/rename_symbols.py" "# tampered\n")
add_test(NAME bundled-sdl-files-tampered
  COMMAND ${CMAKE_COMMAND} -DBUNDLE_DIR=${tampered_bundle} -DSDL_SOURCE_DIR=${sdl3_SOURCE_DIR}
          -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/check-bundled-sdl.cmake
)
# Pass only on the specific failure, so an unrelated error can't satisfy the control.
set_tests_properties(bundled-sdl-files-tampered PROPERTIES
  PASS_REGULAR_EXPRESSION "differs from the SDL release: build-scripts/rename_symbols.py"
)

# The bundled rename scripts run from the skill's layout and leave the traps alone.
find_package(Python3 REQUIRED COMPONENTS Interpreter)
foreach(control IN ITEMS OFF ON)
  set(name rename-smoke)
  if(control)
    set(name rename-smoke-unrenamed)
  endif()
  add_test(NAME ${name}
    COMMAND ${CMAKE_COMMAND} -DPYTHON=${Python3_EXECUTABLE} -DSCRIPTS=${sdl_bundle}/build-scripts
            -DSAMPLE=${CMAKE_CURRENT_SOURCE_DIR}/samples/bounce -DWORK=${CMAKE_CURRENT_BINARY_DIR}/${name}
            -DSKIP_RENAME=${control} -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/rename-smoke.cmake
  )
endforeach()
set_tests_properties(rename-smoke-unrenamed PROPERTIES
  PASS_REGULAR_EXPRESSION "main.c: expected SDL3 include"
)

file(GLOB fixture_dirs LIST_DIRECTORIES true CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/fixtures/*")
foreach(dir IN LISTS fixture_dirs)
  if(NOT IS_DIRECTORY "${dir}")
    continue()
  endif()
  get_filename_component(id "${dir}" NAME)
  foreach(variant IN ITEMS sdl2 naive fixed)
    if(NOT EXISTS "${dir}/${variant}/main.c")
      message(FATAL_ERROR "Trap ${id} has no ${variant}/main.c. Every trap needs sdl2/, naive/ and fixed/.")
    endif()
  endforeach()
  foreach(variant IN ITEMS sdl2 naive fixed)
    add_fixture(${id} ${variant})
  endforeach()
endforeach()

# The SDL2 programs the evals port must build against SDL2, so no case starts
# from broken code. They're interactive, so they're built, not run.
file(GLOB sample_dirs LIST_DIRECTORIES true CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/samples/*")
foreach(dir IN LISTS sample_dirs)
  if(NOT IS_DIRECTORY "${dir}")
    continue()
  endif()
  get_filename_component(name "${dir}" NAME)
  file(GLOB sources CONFIGURE_DEPENDS "${dir}/src/*.c")
  add_executable(sample-${name} ${sources})
  target_link_libraries(sample-${name} PRIVATE SDL2::SDL2main SDL2::SDL2)
  if(MSVC)
    target_compile_options(sample-${name} PRIVATE /W4 /WX)
  else()
    target_compile_options(sample-${name} PRIVATE -Wall -Wextra -Werror)
  endif()
endforeach()

# Positive controls for the naive runner: it must fail on a port that passes and
# on a failure without the trap's line, and it must count a crash as the trap.
function(add_runner_control name target id expected)
  add_test(NAME ${name}
    COMMAND ${CMAKE_COMMAND} -DEXE=$<TARGET_FILE:${target}> -DID=${id} -P ${run_naive}
  )
  set_tests_properties(${name} PROPERTIES
    ENVIRONMENT "SDL_VIDEO_DRIVER=dummy;SDL_AUDIO_DRIVER=dummy"
    PASS_REGULAR_EXPRESSION "${expected}"
    TIMEOUT 30
  )
endfunction()
add_executable(crash cmake/crash.c)
add_runner_control(run-naive-passed bool-returns-fixed bool-returns "the bool-returns trap didn't reproduce")
add_runner_control(run-naive-other-failure bool-returns-naive other-trap "without a line starting with \"other-trap: \"")
add_runner_control(run-naive-crashed crash crash "The naive port crashed, which counts as its trap")

# Every trap card has fixtures, and every fixture folder has a trap card.
set(references "${CMAKE_CURRENT_SOURCE_DIR}/skills/sdl3-porter/references")
set(fixtures "${CMAKE_CURRENT_SOURCE_DIR}/fixtures")
add_test(NAME trap-cards
  COMMAND ${CMAKE_COMMAND} -DREFERENCES=${references} -DFIXTURES=${fixtures}
          -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/check-trap-cards.cmake
)

# Positive controls: the check must fail on a card without fixtures, and on
# fixtures without a card.
set(orphans "${CMAKE_CURRENT_BINARY_DIR}/trap-cards-orphans")
file(REMOVE_RECURSE "${orphans}")
file(COPY "${references}/" DESTINATION "${orphans}/references")
file(APPEND "${orphans}/references/init.md" "\n## orphan-card\n\n**What compiles:** nothing yet.\n")
foreach(dir IN LISTS fixture_dirs)
  get_filename_component(id "${dir}" NAME)
  file(MAKE_DIRECTORY "${orphans}/fixtures/${id}")
endforeach()
file(MAKE_DIRECTORY "${orphans}/fixtures/orphan-fixture")
add_test(NAME trap-cards-orphan-card
  COMMAND ${CMAKE_COMMAND} -DREFERENCES=${orphans}/references -DFIXTURES=${fixtures}
          -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/check-trap-cards.cmake
)
add_test(NAME trap-cards-orphan-fixture
  COMMAND ${CMAKE_COMMAND} -DREFERENCES=${references} -DFIXTURES=${orphans}/fixtures
          -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/check-trap-cards.cmake
)
set_tests_properties(trap-cards-orphan-card PROPERTIES
  PASS_REGULAR_EXPRESSION "trap card without a fixture folder: orphan-card"
)
set_tests_properties(trap-cards-orphan-fixture PROPERTIES
  PASS_REGULAR_EXPRESSION "fixture folder without a trap card: orphan-fixture"
)
