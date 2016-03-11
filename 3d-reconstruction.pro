
QT += core gui opengl

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = 3d-reconstruction
TEMPLATE = app

SOURCES += \
	src/main.cpp \ 
	src/viewport.cpp \
	src/mainwindow.cpp \
        src/tools-widget.cpp \
	src/any.cpp \
	src/mesh.cpp \
	src/algebra.cpp \
	src/trackball.cpp \
	src/points-mover.cpp \
	src/model-creator.cpp \
	src/default-points-mover.cpp \
	src/cylindical-model-creator.cpp \
	src/session.cpp \
	src/timer.cpp \
        src/symmetric-points-mover.cpp

INCLUDEPATH = include
	
HEADERS += \
	include/any.h \
	include/defs.h \
	include/aabb.h \
	include/vec2.h \
	include/vec3.h \
	include/matrix.h \
	include/algebra.h \
	include/lsm.h \
	include/line.h \
	include/plain.h \
	include/triang.h \
	include/triangle.h \
	include/misc.h \
	include/mesh.h \
	include/image.h \
	include/points-mover.h \
	include/model-creator.h \
	include/ellipse-creator.h \
	include/default-points-mover.h \
	include/cylindical-model-creator.h \
        include/tools-widget.h \
	include/mainwindow.h \
	include/trackball.h \
	include/viewport.h \
	include/session.h \
	include/timer.h \
        include/symmetric-points-mover.h
		
CONFIG += c++11
