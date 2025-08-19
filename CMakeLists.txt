cmake_minimum_required(VERSION 3.16)
project(LayOut VERSION 1.0 LANGUAGES C CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

find_package(QT NAMES Qt5 Qt6 REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Gui)
find_package(Qt${QT_VERSION_MAJOR} OPTIONAL_COMPONENTS Widgets)

#qt_standard_project_setup()

add_executable(LayOut
    mygraphicsview.cpp mygraphicsview.h
    main.cpp
    mainwindow.cpp mainwindow.h mainwindow.ui
)
target_link_libraries(LayOut PRIVATE
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
)

#install(TARGETS LayOut
#    BUNDLE DESTINATION .
#    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
#)

#qt_generate_deploy_app_script(
#    TARGET LayOut
#    FILENAME_VARIABLE deploy_script
#    NO_UNSUPPORTED_PLATFORM_ERROR
#)
#install(SCRIPT ${deploy_script})
