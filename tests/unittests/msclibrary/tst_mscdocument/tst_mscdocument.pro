include(../../../../esa.pri)
include(../msclibrarytests.pri)

QT += testlib
QT -= gui

CONFIG += qt console warn_on depend_includepath testcase
CONFIG -= app_bundle

TEMPLATE = app
TARGET = tst_mscdocument

SOURCES +=  tst_mscdocument.cpp
