#-------------------------------------------------
#
# Project created by QtCreator 2015-11-29T19:37:35
#
#-------------------------------------------------

QT       += core gui multimedia

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = pointmetry
TEMPLATE = app

SOURCES +=  main.cpp\
            mainwindow.cpp \
            qimagewidget.cpp \
            qvideocapture.cpp \
            qvideoslider.cpp

HEADERS  += mainwindow.h \
            qimagewidget.h \
            qvideocapture.h \
            qvideoslider.h

FORMS    += mainwindow.ui

include(opencv.pri)
include(opengl.pri)
