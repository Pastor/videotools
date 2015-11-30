#-------------------------------------------------
#
# Project created by QtCreator 2015-11-29T19:37:35
#
#-------------------------------------------------

QT       += core gui multimedia

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Pointmetry
VERSION = 1.0.0.0
TEMPLATE = app

DEFINES += APP_NAME=\\\"$$TARGET\\\" \
           APP_DESIGNER=\\\"pi-null-mezon\\\" \
           APP_VERSION=\\\"$$VERSION\\\" \

SOURCES +=  main.cpp\
            mainwindow.cpp \
            qimagewidget.cpp \
            qvideocapture.cpp \
            qvideoslider.cpp \
    qstasm.cpp

HEADERS  += mainwindow.h \
            qimagewidget.h \
            qvideocapture.h \
            qvideoslider.h \
    qstasm.h

FORMS    += mainwindow.ui

include(opencv.pri)
include(opengl.pri)
include(stasm.pri)
