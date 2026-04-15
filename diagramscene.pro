QT += widgets
requires(qtConfig(fontcombobox))

HEADERS	    =   mainwindow.h \
                dag.h \
                diagramcontroller.h \
		diagramitem.h \
		diagramscene.h \
                arrow.h \
                graph.h \
                operations.h
SOURCES	    =   mainwindow.cpp \
                dag.cpp \
                diagramcontroller.cpp \
		diagramitem.cpp \
                graph.cpp \
		main.cpp \
                arrow.cpp \
		diagramscene.cpp \
                operations.cpp
RESOURCES   =	diagramscene.qrc


# install
target.path = $$[QT_INSTALL_EXAMPLES]/widgets/graphicsview/diagramscene
INSTALLS += target
