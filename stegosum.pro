#-------------------------------------------------
#
# Project created by QtCreator 2012-02-05T16:45:38
#
#-------------------------------------------------

QT       += core gui xml svg

TARGET = stegosum
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    secretmsgtextedit.cpp \
    clickablelabel.cpp \
    analysis/samplepairs.cpp \
    analysis/analysis.cpp \
    analysis/rs.cpp \
    utils.cpp \
    stegosum.cpp \
    raster.cpp \
    vector.cpp \
    variation.cpp

HEADERS  += mainwindow.h \
    secretmsgtextedit.h \
    clickablelabel.hpp \
    analysis/samplepairs.h \
    analysis/analysis.h \
    analysis/rs.h \
    utils.hpp \
    stegosum.h \
    raster.h \
    vector.h \
    variation.h

FORMS    += mainwindow.ui

CONFIG += debug

INCLUDEPATH += /usr/include/QtCrypto

LIBS += -lqca
