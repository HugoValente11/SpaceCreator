include(../msclibrarytests.pri)

QT += testlib
QT -= gui

CONFIG += qt console warn_on depend_includepath testcase
CONFIG -= app_bundle

TEMPLATE = app
TARGET = tst_mscfile

SOURCES +=  tst_mscfile.cpp

win32 {
    DEFINES += EXAMPLES_DIR="\\\"../../../../examples/\\\""
} else {
    DEFINES += EXAMPLES_DIR='\'"../../../../examples/"\''
}

DISTFILES += \
    syntax_error.msc

win32: QMAKE_LFLAGS_RELEASE += /LTCG
