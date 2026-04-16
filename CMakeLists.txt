cmake_minimum_required(VERSION 3.10)
project(DAG)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

#set(Qt5_DIR "C:/Qt/Qt5.14.2/5.14.2/mingw73_64/lib/cmake/Qt5")

find_package(Qt5 REQUIRED COMPONENTS Core Widgets)

set(CMAKE_AUTOMOC ON)

set(SOURCES
    src/main.cpp
    src/mainwindow.cpp
    src/graph.cpp
    src/diagramscene.cpp
    src/visualnode.cpp
    src/arrow.cpp
)

set(HEADERS
    src/mainwindow.h
    src/graph.h
    src/diagramscene.h
    src/visualnode.h
    src/arrow.h
)

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

target_link_libraries(${PROJECT_NAME}
    Qt5::Core
    Qt5::Widgets
)
