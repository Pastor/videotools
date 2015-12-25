#-------------------------------------------------
#
# Project created by QtCreator 2015-11-29T19:37:35
#
#-------------------------------------------------

QT       += core gui multimedia

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = PointmetryOS
VERSION = 1.0.1.1
TEMPLATE = app

DEFINES += APP_NAME=\\\"$$TARGET\\\" \
           APP_DESIGNER=\\\"pi-null-mezon\\\" \
           APP_VERSION=\\\"$$VERSION\\\" \

SOURCES +=  main.cpp\
            mainwindow.cpp \
            qimagewidget.cpp \
            qvideocapture.cpp \
            qvideoslider.cpp \
            qstasm.cpp \
            qeasyplot.cpp \
            qopencvprocessor.cpp \
            qvideowriter.cpp \
            videodialog.cpp

HEADERS  += mainwindow.h \
            qimagewidget.h \
            qvideocapture.h \
            qvideoslider.h \
            qstasm.h \
            qeasyplot.h \
            qopencvprocessor.h \
            qvideowriter.h \
            videodialog.h

FORMS    += mainwindow.ui \
            videodialog.ui

#CONFIG += designbuild # Specifies that project will be build with design path

include(opencv.pri)
include(opengl.pri)
include(stasm.pri)
include(opencl.pri)

RC_ICONS = $${PWD}/../Resources/worker.ico

!designbuild: CONFIG(release, debug|release): DEFINES += QT_NO_WARNING_OUTPUT



