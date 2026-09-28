# Runs the bundled SDL rename scripts on a copy of the bounce sample, from the
# skill's own layout, and checks what they change and what they leave alone.
#
#   cmake -DPYTHON=<exe> -DSCRIPTS=<dir> -DSAMPLE=<dir> -DWORK=<dir> [-DSKIP_RENAME=ON] -P rename-smoke.cmake
#
# SKIP_RENAME=ON is the positive control: the checks must fail on the unrenamed sample.

cmake_minimum_required(VERSION 3.25)

foreach(var IN ITEMS PYTHON SCRIPTS SAMPLE WORK)
  if(NOT DEFINED ${var})
    message(FATAL_ERROR "${var} is not set")
  endif()
endforeach()

file(REMOVE_RECURSE "${WORK}")
file(COPY "${SAMPLE}/src" DESTINATION "${WORK}")

function(run_script script)
  execute_process(
    COMMAND "${PYTHON}" "${SCRIPTS}/${script}" ${ARGN} "${WORK}/src"
    RESULT_VARIABLE failed
    OUTPUT_QUIET
  )
  if(failed)
    message(FATAL_ERROR "${script} failed (${failed})")
  endif()
endfunction()

# The order the skill uses: headers, then symbols, then macros.
if(NOT SKIP_RENAME)
  run_script(rename_headers.py)
  run_script(rename_symbols.py --all-symbols)
  run_script(rename_macros.py)
endif()

file(READ "${WORK}/src/main.c" main)
file(READ "${WORK}/src/audio.c" audio)

set(problems "")
function(expect text what file)
  string(FIND "${${file}}" "${text}" at)
  if(at EQUAL -1)
    set(problems "${problems}\n  ${file}.c: expected ${what}: ${text}" PARENT_SCOPE)
  endif()
endfunction()

# Renamed by the scripts.
expect("#include <SDL3/SDL.h>" "SDL3 include" main)
expect("#include <SDL3/SDL.h>" "SDL3 include" audio)
expect("case SDL_EVENT_GAMEPAD_ADDED:" "renamed event" main)
expect("SDL_RenderTexture(renderer, sprite, NULL, &ball);" "renamed function" main)
expect("want.format = SDL_AUDIO_S16;" "renamed audio format" audio)

# Left alone by the scripts. These are what the skill's trap sweep is for.
expect("SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_GAMEPAD) < 0)" "the bool-returns trap, untouched" main)
expect("SDL_MIX_MAXVOLUME / 2)" "the mix-volume-float trap, untouched" audio)

if(problems)
  message(FATAL_ERROR "The rename scripts didn't do what the skill expects:${problems}")
endif()
message(STATUS "The rename scripts renamed the sample and left its traps for the skill")
