cmake_minimum_required(VERSION 3.10)
project(DAG)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(Qt5_DIR "C:/Qt/Qt5.14.2/5.14.2/mingw73_64/lib/cmake/Qt5")

find_package(Qt5 REQUIRED COMPONENTS Core Widgets)

set(CMAKE_AUTOMOC ON)

set(SOURCES
    main.cpp
    mainwindow.cpp
    graph.cpp
    diagramscene.cpp
    visualnode.cpp
    arrow.cpp
)

set(HEADERS
    mainwindow.h
    graph.h
    diagramscene.h
    visualnode.h
    arrow.h
)

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

target_link_libraries(${PROJECT_NAME}
    Qt5::Core
    Qt5::Widgets
)