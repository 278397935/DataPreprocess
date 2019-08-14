#-------------------------------------------------
#
# Project created by QtCreator 2016-03-30T15:21:44
#
#-------------------------------------------------

QT       += core gui

QT       += sql

CONFIG   += C++11

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = DataPreprocess
TEMPLATE = app


SOURCES += main.cpp \
    Picker/MarkerPicker.cpp \
    mainwindow.cpp \
    Data/RX.cpp \
    Picker/canvaspicker.cpp \
    CalRhoThread.cpp \
    MyDatabase.cpp
HEADERS  += \
    Common/PublicDef.h \
    Picker/MarkerPicker.h \
    mainwindow.h \
    Data/RX.h \
    Picker/canvaspicker.h \
    CalRhoThread.h \
    MyDatabase.h
FORMS    += \
    mainwindow.ui

RESOURCES += \
    Res/Icon.qrc

DEFINES     += QT_DLL QWT_DLL

LIBS += -L "C:\Qt\Qt5.7.0\5.7\mingw53_32\lib" -lqwt

INCLUDEPATH += "C:\Qt\Qt5.7.0\5.7\mingw53_32\include\Qwt"
