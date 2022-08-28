set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(SOURCE_FILES
    MainWindow.h MainWindow.cpp
    pggui.cpp
    ImageDrawWidget.h ImageDrawWidget.cpp
    resources/about.qrc
    resources/icon.qrc
)

find_package(Qt5 COMPONENTS Widgets)

if(NOT Qt5Widgets_FOUND) # Don't add the GUI target if we cannot find the necessary dependencies
    return()
endif()

set(APP_NAME PhotoGoodyzer)

if(WIN32)
    set(APP_ICON_RESOURCE_WINDOWS "${CMAKE_CURRENT_SOURCE_DIR}/resources/icon.rc")
    add_executable(${APP_NAME} WIN32 ${SOURCE_FILES}
        ${APP_ICON_RESOURCE_WINDOWS})
elseif(APPLE)
    set(MACOSX_BUNDLE_ICON_FILE icon.icns)
    set(APP_ICON_MACOSX "${CMAKE_CURRENT_SOURCE_DIR}/resources/icon.icns")
    set_source_files_properties(${APP_ICON_MACOSX} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
    add_executable(${APP_NAME} MACOSX_BUNDLE ${SOURCE_FILES} ${APP_ICON_MACOSX})
else()
    add_executable(${APP_NAME} ${SOURCE_FILES})
endif()

target_link_libraries(${APP_NAME} PRIVATE pglib Qt5::Widgets)

if(APPLE)
    install(TARGETS ${APP_NAME}
        BUNDLE DESTINATION "${CMAKE_BINARY_DIR}")
else()
    INSTALL(TARGETS ${APP_NAME})
endif()
