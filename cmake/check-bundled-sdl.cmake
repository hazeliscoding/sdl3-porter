# Checks that the SDL files bundled with the skill are byte-identical to the
# SDL release the fixtures build against, and that the bundle holds nothing else.
#
#   cmake -DBUNDLE_DIR=<dir> -DSDL_SOURCE_DIR=<dir> -P check-bundled-sdl.cmake

cmake_minimum_required(VERSION 3.25)

foreach(var IN ITEMS BUNDLE_DIR SDL_SOURCE_DIR)
  if(NOT DEFINED ${var})
    message(FATAL_ERROR "${var} is not set")
  endif()
endforeach()

# SOURCE lists the bundled paths after the "Files:" line, up to a blank line.
file(STRINGS "${BUNDLE_DIR}/SOURCE" lines)
set(listed "")
set(in_list FALSE)
foreach(line IN LISTS lines)
  if(line STREQUAL "Files:")
    set(in_list TRUE)
  elseif(in_list AND line STREQUAL "")
    break()
  elseif(in_list)
    list(APPEND listed "${line}")
  endif()
endforeach()
if(NOT listed)
  message(FATAL_ERROR "${BUNDLE_DIR}/SOURCE lists no files")
endif()

set(problems "")
foreach(path IN LISTS listed)
  if(NOT EXISTS "${BUNDLE_DIR}/${path}")
    list(APPEND problems "missing from the bundle: ${path}")
  elseif(NOT EXISTS "${SDL_SOURCE_DIR}/${path}")
    list(APPEND problems "not in the SDL release: ${path}")
  else()
    execute_process(
      COMMAND "${CMAKE_COMMAND}" -E compare_files "${BUNDLE_DIR}/${path}" "${SDL_SOURCE_DIR}/${path}"
      RESULT_VARIABLE differs
    )
    if(differs)
      list(APPEND problems "differs from the SDL release: ${path}")
    endif()
  endif()
endforeach()

file(GLOB_RECURSE present RELATIVE "${BUNDLE_DIR}" "${BUNDLE_DIR}/*")
foreach(path IN LISTS present)
  if(NOT path STREQUAL "SOURCE" AND NOT path IN_LIST listed)
    list(APPEND problems "not listed in SOURCE: ${path}")
  endif()
endforeach()

if(problems)
  list(JOIN problems "\n  " report)
  message(FATAL_ERROR "Bundled SDL files don't match the release:\n  ${report}")
endif()
list(LENGTH listed count)
message(STATUS "${count} bundled SDL files match the release")
