include(../../../../esa.pri)
include(../mscwidgetstests.pri)

QT += widgets testlib

CONFIG += qt console warn_on depend_includepath testcase
CONFIG -= app_bundle

TEMPLATE = app
TARGET = tst_instanceitem

SOURCES +=  tst_instanceitem.cpp
