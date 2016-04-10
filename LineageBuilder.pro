QT += core
QT -= gui

TARGET = LineageBuilder
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += main.cpp \
    array.cpp \
    quadprog.cpp \
    uquadprog.cpp \
    bitree.cpp \
    paintbitree.cpp

HEADERS += \
    array.h \
    quadprog.h \
    uquadprog.h \
    bitree.h \
    common.h \
    paintbitree.h

