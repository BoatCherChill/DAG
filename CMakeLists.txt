cmake_minimum_required(VERSION 3.10)
project(DAG)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt5 REQUIRED COMPONENTS Core Widgets)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(SOURCES
    main.cpp
    mainwindow.cpp
    graph.cpp
    arrow.cpp
    DiagramScene.cpp
    VisualNode.cpp
)

set(HEADERS
    mainwindow.h
    graph.h
    Arrow.h
    DiagramScene.h
    VisualNode.h
)

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

target_link_libraries(${PROJECT_NAME}
    Qt5::Core
    Qt5::Widgets
)
