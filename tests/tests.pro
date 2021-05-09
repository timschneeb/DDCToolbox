QT += testlib
QT += gui
CONFIG += qt warn_on depend_includepath testcase

TEMPLATE = app

SOURCES +=  \
    tst_biquad_operation.cpp

CONFIG += c++17
QTPLUGIN += qsvg

include($$PWD/../src/DDCToolbox-lib.pri)
