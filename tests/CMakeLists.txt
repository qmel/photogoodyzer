set(SOURCE_FILES
    tests.cpp
)

get_filename_component(INCLUDE_DIR "${PROJECT_SOURCE_DIR}/include" REALPATH)
get_filename_component(LIB_DIR "${PROJECT_SOURCE_DIR}/lib" REALPATH)

# find_package(Catch2 REQUIRED)
# add_library(Catch2 INTERFACE)
# target_include_directories(Catch2 INTERFACE "${LIB_DIR}")
add_executable(tests ${SOURCE_FILES})

# target_link_libraries(tests PRIVATE Catch2::Catch2)
# target_link_libraries(tests PRIVATE Catch2)
target_include_directories(tests PRIVATE "${LIB_DIR}" "${INCLUDE_DIR}")
target_link_libraries(tests PRIVATE pglib)

# set_target_properties(tests PROPERTIES EXCLUDE_FROM_ALL YES)
add_custom_command(TARGET tests 
    COMMAND tests
    COMMENT "Running tests"
    POST_BUILD
)
