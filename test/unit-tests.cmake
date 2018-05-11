#
# Unit test compilation routines
# 
# Copyright (C) 2017 Arthur M
#

option(DO_TESTS "Set if you want to enable unit tests, unset if you don't" ON)

if (DO_TESTS)
  enable_testing()
  
  configure_file(${CMAKE_SOURCE_DIR}/test/unit-tests.cmake.in googletest-download/CMakeLists.txt)
  execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
  RESULT_VARIABLE result
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/googletest-download )

if(result)
  message(FATAL_ERROR "CMake step for googletest failed: ${result}")
endif()

execute_process(COMMAND ${CMAKE_COMMAND} --build .
  RESULT_VARIABLE result
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/googletest-download )

if(result)
  message(FATAL_ERROR "Build step for googletest failed: ${result}")
endif()

# Prevent overriding the parent project's compiler/linker
# settings on Windows
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

# Add googletest directly to our build. This defines
# the gtest and gtest_main targets.
add_subdirectory(${CMAKE_SOURCE_DIR}/test/googletest
	${CMAKE_BINARY_DIR}/googletest-build)
  
  file( GLOB SRC_TEST_FILES "${CMAKE_SOURCE_DIR}/test/*.cpp" )

  # Remove Server.cpp (the one with main) from being compiled with the tests
  set(SRC_SERVER_TEST "${SRC_SERVER}")
  list(REMOVE_ITEM SRC_SERVER_TEST "${CMAKE_SOURCE_DIR}/server/Server.cpp" "${CMAKE_SOURCE_DIR}/server/server.cpp")
   message("${SRC_SERVER_TEST}")
 
  add_executable(familyline-tests ${SRC_TEST_FILES} ${SRC_LOGIC} ${SRC_GFX} ${SRC_GUI} ${SRC_INPUT} ${SRC_SERVER_TEST}
	  ${SRC_COMMON}  "${CMAKE_SOURCE_DIR}/src/Timer.cpp") 

  include_directories("${CMAKE_SOURCE_DIR}/src")
  include_directories("${CMAKE_SOURCE_DIR}/server")
  include_directories("${CMAKE_SOURCE_DIR}/common")
  
  file(COPY "${CMAKE_SOURCE_DIR}/test" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/Debug")
 

  option(DO_CHECK_ASAN "Enable address sanitizer" OFF)
  option(SET_COVERAGE "Enable coverage testing" OFF)

  if (SET_COVERAGE)
    target_compile_options(familyline-tests PUBLIC "-fprofile-arcs")
    target_compile_options(familyline-tests PUBLIC "-ftest-coverage")
    target_compile_options(familyline-tests PUBLIC "-O0")
    target_link_libraries(familyline-tests "-fprofile-arcs")
    target_link_libraries(familyline-tests "-ftest-coverage")
    target_link_libraries(familyline-tests "gcov")
    target_link_libraries(familyline-tests "--coverage")
  endif()

  if (DO_CHECK_ASAN)
    target_compile_options(familyline-tests PUBLIC "-fsanitize=address")
    target_link_libraries( familyline-tests
	    gtest gtest_main "-fsanitize=address")
  else()
    target_link_libraries( familyline-tests
      gtest gtest_main)
  endif()

  add_test(NAME general-test COMMAND familyline-tests)
endif(DO_TESTS)


