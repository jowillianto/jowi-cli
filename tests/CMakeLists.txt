# file (
#   GLOB ${PROJECT_NAME}_test_src
#   ${CMAKE_CURRENT_LIST_DIR}/*.cc
# )

# foreach(file IN LISTS ${PROJECT_NAME}_test_src)
#   get_filename_component(file_name ${file} NAME_WE)
#   jowi_add_test(
#     ${PROJECT_NAME}_${file_name}
#     ${file}
#     LIBRARIES ${PROJECT_NAME}
#     SANITIZERS thread address undefined
#   )
# endforeach()
jowi_add_test(
  ${PROJECT_NAME}_argument_parser
  ${CMAKE_CURRENT_LIST_DIR}/argument_parser.cc
  LIBRARIES ${PROJECT_NAME}
  SANITIZERS all
)

jowi_add_test(
  ${PROJECT_NAME}_app_version
  ${CMAKE_CURRENT_LIST_DIR}/app_version.cc
  LIBRARIES ${PROJECT_NAME}
  SANITIZERS all
)