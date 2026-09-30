# Runs a naive fixture and passes only if it fails the way its trap does: it
# exits non-zero after printing a line that starts with "<id>: ", or it crashes.
# A naive port that passes, hangs or fails some other way fails the test.
#
#   cmake -DEXE=<path> -DID=<trap id> -P run-naive.cmake

cmake_minimum_required(VERSION 3.25)

foreach(var IN ITEMS EXE ID)
  if(NOT DEFINED ${var})
    message(FATAL_ERROR "${var} is not set")
  endif()
endforeach()

# CTest can't count a crash as an expected failure, so the fixture runs here.
# The timeout is shorter than the test's, so a hang is reported and killed here.
execute_process(
  COMMAND "${EXE}"
  RESULT_VARIABLE result
  OUTPUT_VARIABLE output
  ERROR_VARIABLE output
  TIMEOUT 20
)
if(output)
  message("${output}")
endif()

if(result MATCHES "timeout")
  message(FATAL_ERROR "The naive port hung instead of failing: ${result}")
elseif(NOT result MATCHES "^-?[0-9]+$")
  message(STATUS "The naive port crashed, which counts as its trap: ${result}")
elseif(result EQUAL 0)
  message(FATAL_ERROR "The naive port passed, so the ${ID} trap didn't reproduce")
elseif(NOT output MATCHES "(^|\n)${ID}: ")
  message(FATAL_ERROR "The naive port exited ${result} without a line starting with \"${ID}: \", so something other than the trap failed")
else()
  message(STATUS "The naive port failed with its trap, as expected")
endif()
