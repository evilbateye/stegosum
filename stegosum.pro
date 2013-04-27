#-------------------------------------------------
#
# Project created by QtCreator 2012-02-05T16:45:38
#
#-------------------------------------------------

QT       += core gui

TARGET = stegosum
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    secretmsgtextedit.cpp \
    pointgenthread.cpp \
    clickablelabel.cpp \
    analysis/samplepairs.cpp \
    analysis/analysis.cpp

HEADERS  += mainwindow.h \
    secretmsgtextedit.h \
    pointgenthread.h \
    clickablelabel.hpp \
    analysis/samplepairs.h \
    analysis/analysis.h

FORMS    += mainwindow.ui

CONFIG += debug

INCLUDEPATH += /usr/include/QtCrypto

LIBS += -lqca
