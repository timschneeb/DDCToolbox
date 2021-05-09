QT += testlib
QT += gui
CONFIG += qt warn_on depend_includepath testcase

TEMPLATE = app

SOURCES +=  \
    tst_biquad_operation.cpp


CONFIG += c++17
QTPLUGIN += qsvg
VERSION = 1.4.0.0

include($$PWD/../src/DDCToolbox-lib.pri)

win32 {
    RC_ICONS = $$PWD/../res/img/icon.ico
    CONFIG(static) {
       DEFINES += WIN_STATIC
       LIBS += $$[QT_INSTALL_LIBS]/../plugins/imageformats/qsvg.lib
    }
}

ICON = $$PWD/../res/img/icon.icns
QMAKE_INFO_PLIST = $$PWD/../deployment/Info.plist
