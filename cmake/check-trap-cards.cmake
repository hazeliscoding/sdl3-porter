# Checks that every trap card has a fixture folder and every fixture folder has
# a trap card. A trap card is a "## <id>" section of a reference file with a
# "**What compiles:**" part.
#
#   cmake -DREFERENCES=<dir> -DFIXTURES=<dir> -P check-trap-cards.cmake

cmake_minimum_required(VERSION 3.25)

foreach(var IN ITEMS REFERENCES FIXTURES)
  if(NOT DEFINED ${var})
    message(FATAL_ERROR "${var} is not set")
  endif()
endforeach()

set(cards "")
file(GLOB reference_files "${REFERENCES}/*.md")
foreach(path IN LISTS reference_files)
  file(READ "${path}" text)
  # Semicolons and brackets would break the list of sections, and ids use neither.
  foreach(char IN ITEMS ";" "[" "]")
    string(REPLACE "${char}" "" text "${text}")
  endforeach()
  string(REPLACE "\n## " ";" sections "${text}")
  foreach(section IN LISTS sections)
    string(FIND "${section}" "**What compiles:**" at)
    if(NOT at EQUAL -1)
      string(REGEX MATCH "^[^\n]*" id "${section}")
      string(STRIP "${id}" id)
      list(APPEND cards "${id}")
    endif()
  endforeach()
endforeach()

set(fixtures "")
file(GLOB entries LIST_DIRECTORIES true RELATIVE "${FIXTURES}" "${FIXTURES}/*")
foreach(name IN LISTS entries)
  if(IS_DIRECTORY "${FIXTURES}/${name}")
    list(APPEND fixtures "${name}")
  endif()
endforeach()

set(problems "")
foreach(id IN LISTS cards)
  if(NOT id IN_LIST fixtures)
    list(APPEND problems "trap card without a fixture folder: ${id}")
  endif()
endforeach()
foreach(id IN LISTS fixtures)
  if(NOT id IN_LIST cards)
    list(APPEND problems "fixture folder without a trap card: ${id}")
  endif()
endforeach()

if(problems)
  list(JOIN problems "\n  " report)
  message(FATAL_ERROR "Trap cards and fixtures don't match:\n  ${report}")
endif()
list(LENGTH cards count)
message(STATUS "${count} trap cards match their fixture folders")
