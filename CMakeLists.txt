cmake_minimum_required(VERSION 3.15)

# enable MSVC_RUNTIME_LIBRARY target property. See https://cmake.org/cmake/help/latest/policy/CMP0091.html
if(POLICY CMP0091)
    cmake_policy(SET CMP0091 NEW)
endif()

project(PhotoGoodyzer 
    DESCRIPTION "A lightweight C++ image manipulation library and CLI, GUI applications"
    VERSION 1.0.1 
    LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED YES)

# Forbid using compiler-specific language extensions.
# https://cmake.org/cmake/help/latest/variable/CMAKE_CXX_EXTENSIONS.html
# https://cmake.org/cmake/help/latest/prop_tgt/CXX_EXTENSIONS.html
set(CMAKE_CXX_EXTENSIONS NO)

option(BUILD_SHARED_LIBS "Build using shared libraries (including CRT for MSVC)" NO)
option(BUILD_DOC "Build documentation (Doxygen required)" YES)
option(BUILD_RUN_TESTS "Build and run tests" YES)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/CMakeModules/")

if(UNIX AND CMAKE_CXX_COMPILER_ID MATCHES "(Clang|^GNU$)")
    if(APPLE AND(CMAKE_SYSTEM_VERSION LESS 19))
        message(FATAL_ERROR "MacOsX prior Catalina (<10.15) are not supported")
    endif()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pedantic -Wall -Wextra -ffast-math")
    option(SAN_ENABLED "Enable the undefined behavior and memory leakage sanitizer when debugging" YES)
    if(SAN_ENABLED)
        set(SAN_FLAGS "-fsanitize=undefined -fsanitize=address -fno-sanitize-recover=all")
        set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${SAN_FLAGS}")
        set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} ${SAN_FLAGS}")
    endif()
elseif(WIN32 AND MSVC)
    if(BUILD_SHARED_LIBS)
        set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
        set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
    else()
        set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    endif()
else()
    message(WARNING "The compiler is not supported (currently): ${CMAKE_CXX_COMPILER_ID}")
endif()

if(BUILD_DOC)
    find_package(Doxygen REQUIRED)
    set(DOXYGEN_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/doc")
    set(DOXYGEN_PROJECT_LOGO "${CMAKE_CURRENT_SOURCE_DIR}/src/pggui/resources/icon64.png")
    set(DOXYGEN_JAVADOC_AUTOBRIEF YES)
    set(DOXYGEN_EXTRACT_ALL YES)
    set(DOXYGEN_DISABLE_INDEX YES)
    set(DOXYGEN_GENERATE_TREE_VIEW YES)
    set(DOXYGEN_WARN_IF_UNDOCUMENTED NO)
    doxygen_add_docs(pgdoc ALL
        ${CMAKE_CURRENT_SOURCE_DIR}/include)
    message(NOTICE "The documentaion will be generated in ${DOXYGEN_OUTPUT_DIRECTORY} folder")
endif()

add_subdirectory(src)

if (BUILD_RUN_TESTS)
    add_subdirectory(tests)
endif()

set(CONFIG_DESTINATION lib/cmake/PhotoGoodyzer)
include(CMakePackageConfigHelpers)

install(EXPORT PhotoGoodyzerTargets
    FILE PhotoGoodyzerTargets.cmake
    NAMESPACE pg::
    DESTINATION "${CONFIG_DESTINATION}")

configure_file(PhotoGoodyzerConfig.cmake.in PhotoGoodyzerConfig.cmake @ONLY)

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/PhotoGoodyzerConfigVersion.cmake"
    VERSION ${CMAKE_PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/PhotoGoodyzerConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/PhotoGoodyzerConfigVersion.cmake"
    DESTINATION "${CONFIG_DESTINATION}")
