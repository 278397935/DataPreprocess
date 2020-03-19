#-------------------------------------------------
#
# Project created by QtCreator 2016-03-30T15:21:44
#
#-------------------------------------------------

QT       += core gui

QT       += sql

CONFIG   += C++11

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = DataPreprocessTang
TEMPLATE = app


SOURCES += main.cpp \
    Mainwindow.cpp \
    Data/RX.cpp \
    Picker/CanvasPicker.cpp \
    Picker/CanvasPickerRho.cpp \
    Picker/MarkerPicker.cpp \
    CalRhoThread.cpp \
    MyDatabase.cpp \
    CustomTableModel.cpp

HEADERS  += \
    Common/PublicDef.h \
    Mainwindow.h \
    Data/RX.h \
    Picker/CanvasPicker.h \
    Picker/CanvasPickerRho.h \
    Picker/MarkerPicker.h \
    CalRhoThread.h \
    MyDatabase.h \
    CustomTableModel.h

FORMS    += \
    Mainwindow.ui

RESOURCES += \
    Res/Icon.qrc


RC_FILE = Res/logo.rc

DEFINES     += QT_DLL QWT_DLL

#LIBS += -L$$PWD/lib/  -l qwtd
#release:　LIBS += -L$$PWD/lib/  -l qwt

LIBS += -L "C:\Qt\Qt5.7.0\5.7\mingw53_32\lib" -lqwt

INCLUDEPATH += "C:\Qt\Qt5.7.0\5.7\mingw53_32\include\Qwt"
