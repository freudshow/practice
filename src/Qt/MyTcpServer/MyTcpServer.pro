#-------------------------------------------------
#
# Project created by QtCreator 2017-03-20T18:54:40
#
#-------------------------------------------------

QT       += core gui
QT       += network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = MyTcpServer
TEMPLATE = app


SOURCES += main.cpp\
        mytcpserver.cpp

HEADERS  += mytcpserver.h

FORMS    += mytcpserver.ui
