# file (
#   GLOB ${PROJECT_NAME}_test_src
#   ${CMAKE_CURRENT_LIST_DIR}/*.cc
# )

# foreach(file IN LISTS ${PROJECT_NAME}_test_src)
#   get_filename_component(file_name ${file} NAME_WE)
#   moderna_add_test(
#     ${PROJECT_NAME}_${file_name}
#     ${file}
#     LIBRARIES ${PROJECT_NAME}
#     SANITIZERS thread address undefined
#   )
# endforeach()
moderna_add_test(
  ${PROJECT_NAME}_argument_parser
  ${CMAKE_CURRENT_LIST_DIR}/argument_parser.cc
  LIBRARIES moderna_cli
  SANITIZERS thread address undefined
)