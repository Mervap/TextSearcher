#-------------------------------------------------
#
# Project created by QtCreator 2019-01-10T20:23:24
#
#-------------------------------------------------

QT       += core gui
QT       += testlib

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = TextSearcher
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++14

SOURCES += \
        main.cpp \
        mainwindow.cpp \
    TrigramCounter/trigramcounter.cpp \
    TrigramContainer/trigramcontainer.cpp \
    TrigramCounter/filetrigramcounter.cpp \
    index.cpp \
    Searcher/searcher.cpp \
    Searcher/filesearcher.cpp \
    Tests/testfilesearcher.cpp \
    Tests/testsearcher.cpp \
    Tests/testfiletrigramcounter.cpp

HEADERS += \
        mainwindow.h \
    TrigramCounter/trigramcounter.h \
    TrigramContainer/trigramcontainer.h \
    TrigramCounter/filetrigramcounter.h \
    index.h \
    Searcher/searcher.h \
    Searcher/filesearcher.h \
    Tests/testfilesearcher.h \
    Tests/testsearcher.h \
    Tests/testfiletrigramcounter.h

FORMS += \
        mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
