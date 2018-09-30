#-------------------------------------------------
#
# Project created by QtCreator 2017-10-02T12:28:51
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = EOSTrigger
TEMPLATE = app


SOURCES += main.cpp\
    gphoto.cpp \
    viewform.cpp

HEADERS  += \
    common.h \
    gphoto.h \
    viewform.h

FORMS    += \
    viewform.ui

unix:!macx: LIBS += -lgphoto2

unix:!macx: LIBS += -lgphoto2_port

RESOURCES += \
    EOSTrigger.qrc
