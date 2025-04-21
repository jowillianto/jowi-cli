cmake_minimum_required(VERSION 3.28)
project (moderna_cli)
set (CMAKE_CXX_STANDARD 23)
set (CMAKE_CXX_STANDARD_REQUIRED true)
set (CMAKE_CXX_SCAN_FOR_MODULES true)

option (MODERNA_CLI_BUILD_TESTS "Build Tests" OFF)

if (NOT TARGET ${PROJECT_NAME}) 
  if (NOT TARGET moderna_generic)
    add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/libs/moderna-generic)
  endif()
  file(GLOB ${PROJECT_NAME}_src
    ${CMAKE_CURRENT_LIST_DIR}/src/*.cc
    ${CMAKE_CURRENT_LIST_DIR}/src/*/*.cc
  )
  add_library(${PROJECT_NAME})
  target_sources(${PROJECT_NAME}
    PUBLIC
      FILE_SET CXX_MODULES FILES ${${PROJECT_NAME}_src}
  )
  target_link_libraries(${PROJECT_NAME} PUBLIC moderna_generic)
endif()

if (MODERNA_CLI_BUILD_TESTS) 
  if (NOT TARGET moderna_test_lib)
    add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/libs/test-lib)
  endif()
  add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/tests)
endif()