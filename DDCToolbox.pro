#-------------------------------------------------
#
# Project created by QtCreator 2019-09-08T00:07:35
#
#-------------------------------------------------

TARGET = DDCToolbox
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

CONFIG += c++17

SOURCES += main.cpp \
    AppRuntime.cpp

QTPLUGIN += qsvg

include($$PWD/DDCToolbox.pri)

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

win32 {
    RC_ICONS = $$PWD/img/icon.ico
    CONFIG(static) {
       DEFINES += WIN_STATIC
       LIBS += $$[QT_INSTALL_LIBS]/../plugins/imageformats/qsvg.lib
    }
}

ICON = $$PWD/img/icon.icns
QMAKE_INFO_PLIST = $$PWD/Info.plist

unix {
   QMAKE_CXXFLAGS += -Wno-unused-variable
}

FORMS +=

HEADERS += \
    AppRuntime.h

DISTFILES += \
    Info.plist
