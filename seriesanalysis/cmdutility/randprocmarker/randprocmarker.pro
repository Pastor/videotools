#-------------------------------------------------
#
# Project created by QtCreator 2015-10-30T22:11:30
#
#-------------------------------------------------
TARGET = randprocmarker
VERSION = 1.0

DEFINES +=  APP_NAME=\\\"$${TARGET}\\\" \
            APP_VERS=\\\"$${VERSION}\\\" \
            APP_DESIGNER=\\\"Alex.A.Taranov\\\"

CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += main.cpp \
           ../../seriesanalyzer.cpp

HEADERS += ../../seriesanalyzer.h

INCLUDEPATH += ../../

mingw: DEFINES += NULL=0
