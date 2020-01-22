#-------------------------------------------------
#
# Project created by QtCreator 2019-09-08T00:07:35
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

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

CONFIG += c++11

SOURCES += \
        biquad.cpp \
        ddccontext.cpp \
        dialog/addpoint.cpp \
        dialog/calc.cpp \
        dialog/customfilterdialog.cpp \
        dialog/shiftfreq.cpp \
        dialog/textpopup.cpp \
        io/conversionengine.cpp \
        io/projectmanager.cpp \
        item/customfilteritem.cpp \
        main.cpp \
        mainwindow.cpp \
        plot/frequencyplot.cpp \
        plot/qcustomplot.cpp \
        tableproxy.cpp \
        undocmd.cpp

HEADERS += \
        biquad.h \
        ddccontext.h \
        delegate.h \
        dialog/addpoint.h \
        dialog/calc.h \
        dialog/customfilterdialog.h \
        dialog/shiftfreq.h \
        dialog/textpopup.h \
        filtertypes.h \
        io/conversionengine.h \
        io/projectmanager.h \
        item/customfilterfactory.h \
        item/customfilteritem.h \
        mainwindow.h \
        plot/frequencyplot.h \
        plot/qcustomplot.h \
        tableproxy.h \
        undocmd.h \
        vdcimporter.h

FORMS += \
        dialog/addpoint.ui \
        dialog/calc.ui \
        dialog/customfilterdialog.ui \
        dialog/shiftfreq.ui \
        dialog/textpopup.ui \
        item/customfilteritem.ui \
        mainwindow.ui

TRANSLATIONS += \
        translations/ddctoolbox_de.ts \
        translations/ddctoolbox_en.ts

include ($$PWD/3rdparty/QSimpleUpdater/QSimpleUpdater.pri)

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    resources.qrc

DISTFILES += \
    translations/ddctoolbox_de.ts \
    translations/ddctoolbox_en.ts
