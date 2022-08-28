set(SOURCE_FILES
    pgcli.cpp
)

add_executable(pgcli ${SOURCE_FILES})

if(WIN32)
    add_compile_definitions(IS_WINDOWS)
endif()

target_link_libraries(pgcli PRIVATE pglib)

install(TARGETS pgcli)
