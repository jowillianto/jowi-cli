cmake_minimum_required(VERSION 3.28)
project (jowi_cli)
set (CMAKE_CXX_STANDARD 23)
set (CMAKE_CXX_STANDARD_REQUIRED true)
set (CMAKE_CXX_SCAN_FOR_MODULES true)

option (JOWI_CLI_BUILD_TESTS "Build Tests" OFF)
option (JOWI_CLI_BENCH_CROGGER "Build the Crogger Benchmarker" OFF)
option (JOWI_CLI_BUILD_EXAMPLES "Build Examples" OFF)

if (NOT TARGET jowi::generic)
    add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/libs/jowi-generic)
endif()

add_library(jowi_tui)
add_library(jowi::tui ALIAS jowi_tui)
target_sources(jowi_tui
  PUBLIC
    FILE_SET CXX_MODULES
      FILES
        "${CMAKE_CURRENT_LIST_DIR}/src/ui/color.cc"
        "${CMAKE_CURRENT_LIST_DIR}/src/ui/main.cc"
        "${CMAKE_CURRENT_LIST_DIR}/src/ui/node.cc"
        "${CMAKE_CURRENT_LIST_DIR}/src/ui/renderer.cc"
        "${CMAKE_CURRENT_LIST_DIR}/src/ui/text_format.cc"
)
target_link_libraries(jowi_tui PUBLIC jowi::generic)
target_compile_features(jowi_tui PUBLIC cxx_std_23)

add_library(${PROJECT_NAME})
add_library(jowi::cli ALIAS ${PROJECT_NAME})
target_sources(${PROJECT_NAME}
  PUBLIC
    FILE_SET CXX_MODULES
      FILES
        "${CMAKE_CURRENT_LIST_DIR}/src/action_builder.cc"
        "${CMAKE_CURRENT_LIST_DIR}/src/app_identity.cc"
        "${CMAKE_CURRENT_LIST_DIR}/src/app_version.cc"
        "${CMAKE_CURRENT_LIST_DIR}/src/app.cc"
        "${CMAKE_CURRENT_LIST_DIR}/src/arg_key.cc"
        "${CMAKE_CURRENT_LIST_DIR}/src/arg_parser.cc"
        "${CMAKE_CURRENT_LIST_DIR}/src/arg_shortcut.cc"
        "${CMAKE_CURRENT_LIST_DIR}/src/arg.cc"
        "${CMAKE_CURRENT_LIST_DIR}/src/env.cc"
        "${CMAKE_CURRENT_LIST_DIR}/src/main.cc"
        "${CMAKE_CURRENT_LIST_DIR}/src/parse_error.cc"
        "${CMAKE_CURRENT_LIST_DIR}/src/parsed_arg.cc"
        "${CMAKE_CURRENT_LIST_DIR}/src/raw_args.cc"
)
target_link_libraries(${PROJECT_NAME} PUBLIC jowi::generic jowi::tui)
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_23)

add_library(jowi_crogger)
add_library(jowi::crogger ALIAS jowi_crogger)
target_sources(jowi_crogger
  PUBLIC
    FILE_SET CXX_MODULES
      FILES
        "${CMAKE_CURRENT_LIST_DIR}/src/crogger/error.cc"
        "${CMAKE_CURRENT_LIST_DIR}/src/crogger/context.cc"
        "${CMAKE_CURRENT_LIST_DIR}/src/crogger/severity.cc"
        "${CMAKE_CURRENT_LIST_DIR}/src/crogger/filter.cc"
        "${CMAKE_CURRENT_LIST_DIR}/src/crogger/formatter.cc"
        "${CMAKE_CURRENT_LIST_DIR}/src/crogger/logger.cc"
        "${CMAKE_CURRENT_LIST_DIR}/src/crogger/emitter.cc"
        "${CMAKE_CURRENT_LIST_DIR}/src/crogger/main.cc"
)
target_link_libraries(jowi_crogger PUBLIC jowi::generic jowi::tui)
target_compile_features(jowi_crogger PUBLIC cxx_std_23)

if (JOWI_CLI_BUILD_TESTS)
    include(CTest)
    if (NOT TARGET jowi::test_lib)
        add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/libs/test-lib)
    endif()
    add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/tests)
endif()

if (JOWI_CLI_BENCH_CROGGER)
    if (NOT TARGET jowi::test_lib)
        add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/libs/test-lib)
    endif()
    add_executable(crogger_benchmark ${CMAKE_CURRENT_LIST_DIR}/benchmarks/crogger.cc)
    target_link_libraries(crogger_benchmark
    PRIVATE
      jowi::crogger
      jowi::cli
      jowi::test_lib
  )
endif()

if (JOWI_INSTALL)
    include(GNUInstallDirs)
    set (JOWI_COMPONENT_NAME "cli")
    set_property(
    TARGET ${PROJECT_NAME}
    PROPERTY EXPORT_NAME ${JOWI_COMPONENT_NAME}
  )
    install (
    TARGETS ${PROJECT_NAME}
    EXPORT jowi
    FILE_SET
      CXX_MODULES
      DESTINATION ${CMAKE_INSTALL_PREFIX}/cxx_modules/jowi/${JOWI_COMPONENT_NAME}
      COMPONENT ${JOWI_COMPONENT_NAME}_file_set
    CXX_MODULES_BMI
      COMPONENT ${JOWI_COMPONENT_NAME}
      DESTINATION ${CMAKE_INSTALL_PREFIX}/cxx_modules/jowi/${JOWI_COMPONENT_NAME}
  )
    if (NOT JOWI_GLOBAL_INSTALL)
        install (
      EXPORT jowi
      DESTINATION ${CMAKE_INSTALL_LIBDIR}/jowi
      FILE jowi-config.cmake
      NAMESPACE jowi::
      CXX_MODULES_DIRECTORY cxx_modules/jowi
    )
    endif()
endif()

if (JOWI_CLI_BUILD_EXAMPLES)
    add_executable("${PROJECT_NAME}_example_echo" "${CMAKE_CURRENT_LIST_DIR}/examples/echo.cc")
    target_link_libraries(${PROJECT_NAME}_example_echo PUBLIC jowi::cli)

    add_executable("${PROJECT_NAME}_example_actions" "${CMAKE_CURRENT_LIST_DIR}/examples/actions.cc")
    target_link_libraries(${PROJECT_NAME}_example_actions PUBLIC jowi::cli)

    add_executable("${PROJECT_NAME}_example_ui" "${CMAKE_CURRENT_LIST_DIR}/examples/ui.cc")
    target_link_libraries(${PROJECT_NAME}_example_ui PUBLIC jowi::cli jowi::tui)

    add_executable("${PROJECT_NAME}_example_csv" "${CMAKE_CURRENT_LIST_DIR}/examples/csv.cc")
    target_link_libraries(${PROJECT_NAME}_example_csv PUBLIC jowi::cli jowi::tui)
endif()
